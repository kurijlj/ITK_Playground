// ============================================================================
// image_registration.cxx (ITK_Playground) - Register two RGB images.
//
//  Copyright (C) 2024 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
//
// "ITK Playground" is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, either version 3 of the License, or (at your option) 
// any later version.
//
// "ITK Playground" is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with Focus Precision Analyze. If not, see <https://www.gnu.org/licenses/>.
// ============================================================================


// ============================================================================
//
// 2024-08-07 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
//
// * image_registration.cxx: created.
//
// ============================================================================


// ============================================================================
// Preprocessor directives section
// ============================================================================


// ============================================================================
// Headers include section
// ============================================================================

// Related header

// "C" headers
#include <cstdlib>                   // required by EXIT_SUCCESS, EXIT_FAILURE

// Standard Library headers
#include <exception>                 // required by std::current_exception
#include <filesystem>                // required by std::filesystem
#include <iostream>                  // required by cin, cout, ...
#include <limits>                    // required by std::numeric_limits
#include <string>                    // required by std::string
#include <vector>                    // required by std::vector

// External libraries headers
#include <clipp.hpp>                 // command line arguments parsing
#include <itkCommand.h>
#include <itkEuler2DTransform.h>
#include <itkImage.h>                // required by itk::Image
#include <itkImageFileReader.h>      // required for the reading image data
#include <itkImageFileWriter.h>      // required for writing image data to file
#include <itkImageRegistrationMethodv4.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkMeanSquaresImageToImageMetricv4.h>
#include <itkRGBToLuminanceImageFilter.h>  // required for converting RGB to
                                           // luminance image
#include <itkRegularStepGradientDescentOptimizerv4.h>
#include <itkResampleImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkSmartPointer.h>         // required by itk::SmartPointer
#include <itkSubtractImageFilter.h>
#include <itkTIFFImageIO.h>          // required for reading and writing
                                     // TIFF images

// ============================================================================
// Global constants section
// ============================================================================

static constexpr auto kAppName = "image_registration";
static constexpr auto kVersionString = "0.1";
static constexpr auto kYearString = "2024";
static constexpr auto kAuthorName = "Ljubomir Kurij";
static constexpr auto kAuthorEmail = "ljubomir_kurij@protonmail.com";
static constexpr auto kAppDoc = "\
Register two RGB images.\n\n\
Mandatory arguments to long options are mandatory for short options too.\n";
static constexpr auto kLicense = "\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";


// ============================================================================
// Global variables section
// ============================================================================


// ============================================================================
// Utility function prototypes
// ============================================================================

void printShortHelp(const char * = kAppName);
void printUsage(
  const clipp::group &,
  const char * = kAppName,
  const clipp::doc_formatting & = clipp::doc_formatting{}
  );
void printVersionInfo(
  const char * = kAppName,
  const char * = kVersionString,
  const char * = kYearString,
  const char * = kAuthorName,
  const char * = kLicense
  );
void showHelp(
  const clipp::group &,
  const char * = kAppName,
  const char * = kAppDoc,
  const char * = kAuthorEmail
  );


// ============================================================================
// Utility class definitions
// ============================================================================

class CommandIterationUpdate : public itk::Command {
public:
  using Self = CommandIterationUpdate;
  using Superclass = itk::Command;
  using Pointer = itk::SmartPointer<Self>;
  itkNewMacro(Self);
 
protected:
  CommandIterationUpdate() = default;
 
public:
  using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
  using OptimizerPointer = const OptimizerType *;
 
  void
  Execute(itk::Object * caller, const itk::EventObject & event) override {
    Execute((const itk::Object *) caller, event);
  }
 
  void
  Execute(const itk::Object * object, const itk::EventObject & event) override {
    auto optimizer = static_cast<OptimizerPointer>(object);
    if (!itk::IterationEvent().CheckEvent(&event)) {
      return;
    }
    std::cout << optimizer->GetCurrentIteration() << "   ";
    std::cout << optimizer->GetValue() << "   ";
    std::cout << optimizer->GetCurrentPosition() << "\n";
  }
};


// ============================================================================
// Main Function Section
// ============================================================================

int main(int argc, char *argv[]) {
  namespace fs = std::filesystem; // Filesystem alias

  // Determine the exec name under wich program is beeing executed
  std::string exec_name = fs::path(argv[0]).filename().string();

  // Here we define the structure for holding the passed command line otions.
  // The structure is also used to define the command line options and their
  // default values.
  struct CLIOptions {
    bool show_help;
    bool print_usage;
    bool show_version;
    std::string fixed_image;
    std::string moving_image;
    bool overwrite;
    std::vector<std::string> unsupported;
  };

  // Define the default values for the command line options
  CLIOptions user_options{
      false,        // show_help
      false,        // print_usage
      false,        // show_version
      "",           // fixed_image
      "",           // moving_image
      false,        // overwrite
      {}            // unsupported options aggregator
  };

  // Option filters definitions
  auto istarget = clipp::match::prefix_not("-"); // Filter out strings that
                                                 // start with '-' (options)

  // Set command line options
  auto parser_config = (
      // Define the command line options and their default values.
      // - Must have more than one option.
      // - The order of the options is important.
      // - The order of the options in the group is important.
      // - Take care not to omitt value filter when parsing file and directory
      //   names. Otherwise, the parser will treat options as values.
      // - Define positional arguments first
      // - Define positional srguments as optional to enforce the priority of
      //   help, usage and version switches. Then enforce the required
      //   positional arguments by checking if their values are set.
      (
        clipp::required("-f", "--fixed-image")
          .doc("fixed image file")
        & clipp::opt_value(istarget, "FIXED_IMAGE", user_options.fixed_image),
        clipp::required("-m", "--moving-image")
          .doc("moving image file")
        & clipp::opt_value(istarget, "MOVING_IMAGE", user_options.moving_image),
        clipp::option("-o", "--overwrite")
          .set(user_options.overwrite)
          .doc("overwrite existing result file"),
        clipp::option("-h", "--help")
           .set(user_options.show_help)
           .doc("show this help message and exit"),
        clipp::option("--usage")
           .set(user_options.print_usage)
           .doc("give a short usage message"),
        clipp::option("-V", "--version")
           .set(user_options.show_version)
           .doc("print program version")
        ).doc("general options:"),
      clipp::any_other(user_options.unsupported));

  // Execute the main code inside a try block to catch any exceptions and
  // to ensure that all of the code exits at exactly the same point
  try {
    // Parse command line options
    auto result = clipp::parse(argc, argv, parser_config);

    // Check if the unsupported options were passed
    if (!user_options.unsupported.empty()) {
      std::cerr << exec_name << ": Unsupported options: ";
      for (const auto &opt : user_options.unsupported) {
        std::cerr << opt << " ";
      }
      std::cerr << "\n";
      printShortHelp(exec_name.c_str());

      throw EXIT_FAILURE;
    }

    // Check if the help switch was triggered. We give help switch the
    // highest priority, so if it is triggered we don't need to check
    // anything else.
    if (user_options.show_help) {
      showHelp(parser_config, exec_name.c_str(), kAppDoc, kAuthorEmail);

      throw EXIT_SUCCESS;
    }

    // Check if the usage switch was triggered. Usge switch has the second
    // highest priority, so if it is triggered we don't need to check
    // anything else.
    if (user_options.print_usage) {
      auto fmt = clipp::doc_formatting{}.first_column(0).last_column(79);
      printUsage(parser_config, exec_name.c_str(), fmt);

      throw EXIT_SUCCESS;
    }

    // Check if the version switch was triggered. Version switch has the
    // third highest priority.
    if (user_options.show_version) {
      printVersionInfo(
        exec_name.c_str(),
        kVersionString,
        kYearString,
        kAuthorName,
        kLicense
        );
      throw EXIT_SUCCESS;
    }

    // No high priority switch was triggered. Now we check if the input
    // files were passed. If not we print the usage message and exit.

    // Check if the fixed image file was passed
    if (user_options.fixed_image.empty()) {
      auto fmt = clipp::doc_formatting {}
        .first_column(0)
        .last_column(79)
        .merge_alternative_flags_with_common_prefix(true);
      std::cout << "Usage: ";
      printUsage(parser_config, exec_name.c_str(), fmt);

      std::cout << "\n";

      // Print short help message
      printShortHelp(exec_name.c_str());

      throw EXIT_FAILURE;
    }

    // Fixed image file was passed. Now we check if the file exists, is
    // readable and is a regular file and not an empty file.
    // Check if the file exists
    if (!fs::exists (user_options.fixed_image)) {
      std::cerr << exec_name
        << ": File does not exist: "
        << user_options.fixed_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if the file is a regular file
    if (!fs::is_regular_file (user_options.fixed_image)) {
      std::cerr << exec_name
        << ": Not a regular file: "
        << user_options.fixed_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if the file is empty
    if (fs::file_size (user_options.fixed_image) == 0) {
      std::cerr << exec_name
        << ": Empty file: "
        << user_options.fixed_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Open the file in binary mode for wider compatibility
    std::ifstream fixed_file (
      user_options.fixed_image,
      std::ios::binary
      );

    // Check if the file was opened successfully
    // (if we can read it)
    if (!fixed_file.is_open()) {
      std::cerr << exec_name
        << ": Error opening file: "
        << user_options.fixed_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Close the file
    fixed_file.close();

    auto tiffImageIO = itk::TIFFImageIO::New();

    // Check if we are dealing with a TIFF image
    if (!tiffImageIO->CanReadFile(user_options.fixed_image.c_str())) {
      std::cerr << exec_name
        << ": Unsupported image format: "
        << user_options.fixed_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Read the image information
    tiffImageIO->SetFileName(user_options.fixed_image);
    tiffImageIO->ReadImageInformation();

    // Check if we are dealing with an compressed image
    if (tiffImageIO->ReadCompressionFromImage() != 1) {  // 1 = no compression
      std::cerr << exec_name
        << ": File is compressed: "
        << user_options.fixed_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if we are dealing with the RGB image
    if (tiffImageIO->ReadSamplesPerPixelFromImage() != 3) {
      std::cerr << exec_name
        << ": File is not an RGB image: "
        << user_options.fixed_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if we are dealing with 16-bit image
    if (tiffImageIO->ReadBitsPerSampleFromImage() != 16) {
      std::cerr << exec_name
        << ": File is not a 16-bit image: "
        << user_options.fixed_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if the moving image file was passed
    if (user_options.moving_image.empty()) {
      auto fmt = clipp::doc_formatting {}
        .first_column(0)
        .last_column(79)
        .merge_alternative_flags_with_common_prefix(true);
      std::cout << "Usage: ";
      printUsage(parser_config, exec_name.c_str(), fmt);

      std::cout << "\n";

      // Print short help message
      printShortHelp(exec_name.c_str());

      throw EXIT_FAILURE;
    }

    // Moving image file was passed. Now we check if the file exists, is
    // readable and is a regular file and not an empty file.
    // Check if the file exists
    if (!fs::exists (user_options.moving_image)) {
      std::cerr << exec_name
        << ": File does not exist: "
        << user_options.moving_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if the file is a regular file
    if (!fs::is_regular_file (user_options.moving_image)) {
      std::cerr << exec_name
        << ": Not a regular file: "
        << user_options.moving_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if the file is empty
    if (fs::file_size (user_options.moving_image) == 0) {
      std::cerr << exec_name
        << ": Empty file: "
        << user_options.moving_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Open the file in binary mode for wider compatibility
    std::ifstream moving_file (
      user_options.moving_image,
      std::ios::binary
      );

    // Check if the file was opened successfully
    // (if we can read it)
    if (!moving_file.is_open()) {
      std::cerr << exec_name
        << ": Error opening file: "
        << user_options.moving_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Close the file
    moving_file.close();

    tiffImageIO = itk::TIFFImageIO::New();

    // Check if we are dealing with a TIFF image
    if (!tiffImageIO->CanReadFile(user_options.moving_image.c_str())) {
      std::cerr << exec_name
        << ": Unsupported image format: "
        << user_options.moving_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Read the image information
    tiffImageIO->SetFileName(user_options.moving_image);
    tiffImageIO->ReadImageInformation();

    // Check if we are dealing with an compressed image
    if (tiffImageIO->ReadCompressionFromImage() != 1) {  // 1 = no compression
      std::cerr << exec_name
        << ": File is compressed: "
        << user_options.moving_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if we are dealing with the RGB image
    if (tiffImageIO->ReadSamplesPerPixelFromImage() != 3) {
      std::cerr << exec_name
        << ": File is not an RGB image: "
        << user_options.moving_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if we are dealing with 16-bit image
    if (tiffImageIO->ReadBitsPerSampleFromImage() != 16) {
      std::cerr << exec_name
        << ": File is not a 16-bit image: "
        << user_options.moving_image
        << "\n";
      throw EXIT_FAILURE;
    }

    // Decompose the moving image file name into the base name and the extension
    std::string out_base_name
      = fs::path(user_options.moving_image).stem().string();
    std::string out_extension
      = fs::path(user_options.moving_image).extension().string();

    // Check if the output file already exists
    if (
        !user_options.overwrite
        && fs::exists (out_base_name + "_registered" + out_extension)
        ) {
      std::cerr << kAppName
        << ": Output file already exists: "
        << out_base_name << "_registered" << out_extension
        << "\n";
      throw EXIT_FAILURE;
    }

    // Main code goes here ----------------------------------------------------

    // Define the pixel and image types
    constexpr unsigned int Dimension = 2;
    using RGB16Pixel = itk::RGBPixel<float>;
    using RGB16Image = itk::Image<RGB16Pixel, Dimension>;
    using Mono16Image = itk::Image<float, Dimension>;

    // Define the image reader and writer types
    using RGB16Reader = itk::ImageFileReader<RGB16Image>;
    using RGB16Writer = itk::ImageFileWriter<RGB16Image>;
    using Mono16Writer = itk::ImageFileWriter<Mono16Image>;

    // Define the luminance filter type
    using LuminanceFilterType
      = itk::RGBToLuminanceImageFilter<RGB16Image, Mono16Image>;

    // Convert the input images to luminance images for easier registration
    auto fixed_reader = RGB16Reader::New();
    fixed_reader->SetFileName(user_options.fixed_image);
    auto fixed_image = RGB16Image::New();
    fixed_image = fixed_reader->GetOutput();
    auto moving_reader = RGB16Reader::New();
    moving_reader->SetFileName(user_options.moving_image);
    auto moving_image = RGB16Image::New();
    moving_image = moving_reader->GetOutput();
    auto fixed_to_luminance = LuminanceFilterType::New();
    fixed_to_luminance->SetInput(fixed_reader->GetOutput());
    auto moving_to_luminance = LuminanceFilterType::New();
    moving_to_luminance->SetInput(moving_reader->GetOutput());
    auto fixed_luminance = Mono16Image::New();
    auto moving_luminance = Mono16Image::New();
    fixed_luminance = fixed_to_luminance->GetOutput();
    moving_luminance = moving_to_luminance->GetOutput();

    try {
      fixed_image->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << exec_name
        << ": Error reading fixed image: '"
        << user_options.fixed_image
        << "'. '"
        << error
        << "\n";
      return EXIT_FAILURE;
    }
    try {
      moving_image->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << exec_name
        << ": Error reading moving image: '"
        << user_options.moving_image
        << "'. '"
        << error
        << "\n";
      return EXIT_FAILURE;
    }
    try {
      fixed_luminance->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << exec_name
        << ": Error converting the fixed image to luminance: '"
        << user_options.fixed_image
        << "'. '"
        << error
        << "\n";
      return EXIT_FAILURE;
    }
    try {
      moving_luminance->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << exec_name
        << ": Error converting the moving image to luminance: '"
        << user_options.fixed_image
        << "'. '"
        << error
        << "\n";
      return EXIT_FAILURE;
    }

    // For testing purposes try to write all four images
    auto rgb_writer = RGB16Writer::New();

    rgb_writer->SetFileName("fixed_image.tif");
    rgb_writer->SetInput(fixed_image);
    // The complete pipeline is executed by invoking Update() on the writer.
    try {
      rgb_writer->Update();
    } catch (const itk::ExceptionObject & error) {
      std::cerr << exec_name
        << ": Error writing file: 'fixed_image.tif'. "
        << error
        << "\n";
      throw EXIT_FAILURE;
    }

    rgb_writer->SetFileName("moving_image.tif");
    rgb_writer->SetInput(moving_image);
    // The complete pipeline is executed by invoking Update() on the writer.
    try {
      rgb_writer->Update();
    } catch (const itk::ExceptionObject & error) {
      std::cerr << exec_name
        << ": Error writing file: 'moving_image.tif'. "
        << error
        << "\n";
      throw EXIT_FAILURE;
    }

    auto mono_writer = Mono16Writer::New();

    mono_writer->SetFileName("fixed_luminance.tif");
    mono_writer->SetInput(fixed_luminance);
    try {
      mono_writer->Update();
    } catch (const itk::ExceptionObject & error) {
      std::cerr << exec_name
        << ": Error writing file: 'fixed_luminance.tif'. "
        << error
        << "\n";
      throw EXIT_FAILURE;
    }

    mono_writer->SetFileName("moving_luminance.tif");
    mono_writer->SetInput(moving_luminance);
    try {
      mono_writer->Update();
    } catch (const itk::ExceptionObject & error) {
      std::cerr << exec_name
        << ": Error writing file: 'moving_luminance.tif'. "
        << error
        << "\n";
      throw EXIT_FAILURE;
    }

    using TransformType = itk::Euler2DTransform<double>;
 
    using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
    using MetricType
      = itk::MeanSquaresImageToImageMetricv4<Mono16Image, Mono16Image>;
    using RegistrationType
      = itk::ImageRegistrationMethodv4<Mono16Image, Mono16Image, TransformType>;
 
    auto metric       = MetricType::New();
    auto optimizer    = OptimizerType::New();
    auto registration = RegistrationType::New();
 
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);

    auto initial_transform = TransformType::New();
 
    registration->SetFixedImage(fixed_luminance);
    registration->SetMovingImage(moving_luminance);

    using SpacingType = RGB16Image::SpacingType;
    using OriginType  = RGB16Image::PointType;
    using RegionType  = RGB16Image::RegionType;
    using SizeType    = RGB16Image::SizeType;
 
    const SpacingType fixed_spacing = fixed_image->GetSpacing();
    const OriginType  fixed_origin  = fixed_image->GetOrigin();
    const RegionType  fixed_region  = fixed_image->GetLargestPossibleRegion();
    const SizeType    fixed_size    = fixed_region.GetSize();
 
    TransformType::InputPointType center_fixed;
 
    center_fixed[0] = fixed_origin[0] + fixed_spacing[0] * fixed_size[0] / 2.0;
    center_fixed[1] = fixed_origin[1] + fixed_spacing[1] * fixed_size[1] / 2.0;
    
    const SpacingType moving_spacing = moving_image->GetSpacing();
    const OriginType  moving_origin  = moving_image->GetOrigin();
    const RegionType  moving_region  = moving_image->GetLargestPossibleRegion();
    const SizeType    moving_size    = moving_region.GetSize();
 
    TransformType::InputPointType center_moving;
 
    center_moving[0]
      = moving_origin[0] + moving_spacing[0] * moving_size[0] / 2.0;
    center_moving[1]
      = moving_origin[1] + moving_spacing[1] * moving_size[1] / 2.0;

    initial_transform->SetCenter(center_fixed);
    initial_transform->SetTranslation(center_moving - center_fixed);
    initial_transform->SetAngle(0.0);
 
    registration->SetInitialTransform(initial_transform);

    using OptimizerScalesType = OptimizerType::ScalesType;
    OptimizerScalesType optimizer_scales(
      initial_transform->GetNumberOfParameters()
      );

    const double translation_scale = 1.0 / 1000.0;
    optimizer_scales[0] = 1.0;
    optimizer_scales[1] = translation_scale;
    optimizer_scales[2] = translation_scale;
 
    optimizer->SetScales(optimizer_scales);   

    double initial_step_length = 0.1;
 
    optimizer->SetRelaxationFactor(0.6);
    optimizer->SetLearningRate(initial_step_length);
    optimizer->SetMinimumStepLength(0.001);
    optimizer->SetNumberOfIterations(200);
 
 
    auto observer = CommandIterationUpdate::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);
 
    // One level registration process without shrinking and smoothing.
    constexpr unsigned int number_of_levels = 1;
 
    RegistrationType::ShrinkFactorsArrayType shrink_factors_per_level;
    shrink_factors_per_level.SetSize(1);
    shrink_factors_per_level[0] = 1;
 
    RegistrationType::SmoothingSigmasArrayType smoothing_sigmas_per_level;
    smoothing_sigmas_per_level.SetSize(1);
    smoothing_sigmas_per_level[0] = 0;
 
    registration->SetNumberOfLevels(number_of_levels);
    registration->SetSmoothingSigmasPerLevel(smoothing_sigmas_per_level);
    registration->SetShrinkFactorsPerLevel(shrink_factors_per_level);
 
    try {
      registration->Update();
      std::cout << exec_name << ": Optimizer stop condition: "
        << registration->GetOptimizer()->GetStopConditionDescription()
        << "\n";
    } catch (const itk::ExceptionObject & error) {
      std::cerr << exec_name << "Error registering images: " << error << "\n";
      return EXIT_FAILURE;
    }
 
    const TransformType::ParametersType final_parameters =
      registration->GetOutput()->Get()->GetParameters();
 
    const double final_angle = final_parameters[0];
    const double final_translation_x = final_parameters[1];
    const double final_translation_y = final_parameters[2];
 
    const double rotation_center_x =
      registration->GetOutput()->Get()->GetCenter()[0];
    const double rotation_center_y =
      registration->GetOutput()->Get()->GetCenter()[1];
 
    const unsigned int number_of_iterations = optimizer->GetCurrentIteration();
    const double best_value = optimizer->GetValue();
    const double final_angle_in_degrees = final_angle * 180.0 / itk::Math::pi;
 
    std::cout << "Result = " << "\n";
    std::cout << " Angle (radians) = " << final_angle << "\n";
    std::cout << " Angle (degrees) = " << final_angle_in_degrees << "\n";
    std::cout << " Translation X   = " << final_translation_x << "\n";
    std::cout << " Translation Y   = " << final_translation_y << "\n";
    std::cout << " Fixed Center X  = " << rotation_center_x << "\n";
    std::cout << " Fixed Center Y  = " << rotation_center_y << "\n";
    std::cout << " Iterations      = " << number_of_iterations << "\n";
    std::cout << " Metric value    = " << best_value << "\n";


    using ResampleFilterType = itk::ResampleImageFilter<RGB16Image, RGB16Image>;
    auto resample = ResampleFilterType::New();
 
    resample->SetTransform(registration->GetTransform());
    resample->SetInput(moving_image);
 
    resample->SetSize(fixed_image->GetLargestPossibleRegion().GetSize());
    resample->SetOutputOrigin(fixed_image->GetOrigin());
    resample->SetOutputSpacing(fixed_image->GetSpacing());
    resample->SetOutputDirection(fixed_image->GetDirection());

    RGB16Image::PixelType default_pixel_value;
    default_pixel_value[0]
      = static_cast<float>(std::numeric_limits<uint16_t>::max());
    default_pixel_value[1]
      = static_cast<float>(std::numeric_limits<uint16_t>::max());
    default_pixel_value[2]
      = static_cast<float>(std::numeric_limits<uint16_t>::max());
    resample->SetDefaultPixelValue(default_pixel_value);
 
    using CastFilterType =
      itk::CastImageFilter<RGB16Image, RGB16Image>;
 
    auto caster = CastFilterType::New();
 
    rgb_writer->SetFileName(out_base_name + "_registered" + out_extension);
 
    caster->SetInput(resample->GetOutput());
    rgb_writer->SetInput(caster->GetOutput());
 
    try {
      rgb_writer->Update();
    } catch (const itk::ExceptionObject & error) {
      std::cerr << exec_name
        << ": Error writing file: '"
        << out_base_name << "_registered" << out_extension
        << "'. " << error << "\n";
      return EXIT_FAILURE;
    }
 
    using DifferenceFilterType
      = itk::SubtractImageFilter<RGB16Image, RGB16Image, RGB16Image>;
 
    auto difference = DifferenceFilterType::New();

    /*
    using RescalerType =
      itk::RescaleIntensityImageFilter<RGB16Image, RGB16Image>;
 
    auto intensity_rescaler = RescalerType::New();
 
    intensity_rescaler->SetOutputMinimum(
      static_cast<float>(std::numeric_limits<uint16_t>::min())
      );
    intensity_rescaler->SetOutputMaximum(
      static_cast<float>(std::numeric_limits<uint16_t>::max())
      );
    */
 
    difference->SetInput1(fixed_image);
    difference->SetInput2(resample->GetOutput());
 
    // intensity_rescaler->SetInput(difference->GetOutput());
 
    rgb_writer->SetFileName("difference.tif");
    // rgb_writer->SetInput(intensity_rescaler->GetOutput());
    rgb_writer->SetInput(difference->GetOutput());
 
    try {
        rgb_writer->Update();
    } catch (const itk::ExceptionObject & error) {
      std::cerr << exec_name
        << ": Error writing file: 'difference.tif'. "
        << error << "\n";
      return EXIT_FAILURE;
    }


    // Return success
    throw EXIT_SUCCESS;

  } catch (int result) {
    // Return the result of the main code
    return result;

  } catch (...) {
    // We have an unhandled exception. Print error message and exit
    try {
      std::rethrow_exception(std::current_exception());
    } catch (const std::exception &e) {
      std::cerr << exec_name << ": Unhandled exception: " << e.what()
                << "\n";
    }

    // Return an error code
    return EXIT_FAILURE;
  }

  // The code should never reach this point. If it does, print an error
  // message and exit
  std::cerr << exec_name << ": Unhandled program exit!" << "\n";

  return EXIT_FAILURE;
}


// ============================================================================
// Function definitions
// ============================================================================

inline void printShortHelp(const char *exec_name) {
  std::cout << "Try '" << exec_name << " --help' for more information.\n";
}

inline void printUsage(
    const clipp::group          &group,
    const char                  *prefix,
    const clipp::doc_formatting &fmt
    ) {
  std::cout
    << clipp::usage_lines(group, std::string {prefix}, fmt)
    << "\n";
}

inline void printVersionInfo(
    const char *exec_name,
    const char *app_version,
    const char *release_year,
    const char *author_name,
    const char *license
) {
  std::cout
    << exec_name << " "
    << app_version << " Copyright (C) "
    << release_year << " "
    << author_name << "\n"
    << license;
}

void showHelp(
    const clipp::group &group,
    const char         *exec_name,
    const char         *doc,
    const char         *author_email
    ) {
  auto fmt = clipp::doc_formatting{}.first_column(0).last_column(79);
  clipp::man_page man;

  man.prepend_section("USAGE",
    clipp::usage_lines(group, std::string {exec_name}, fmt).str());
  man.append_section("", std::string {doc});
  man.append_section("", clipp::documentation(group, fmt).str());
  man.append_section("",
    "Report bugs to <" + std::string {author_email} + ">.");

  std::cout << man;
}

