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
#include <itkAffineTransform.h>
#include <itkCastImageFilter.h>
#include <itkCommandIterationUpdate.h>
#include <itkImage.h>                // required by itk::Image
#include <itkImageFileReader.h>      // required for the reading image data
#include <itkImageFileWriter.h>      // required for writing image data to file
#include <itkImageRegistrationMethod.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkRGBToLuminanceImageFilter.h>  // required for converting RGB to
                                           // luminance image
#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkResampleImageFilter.h>
#include <itkSmartPointer.h>         // required by itk::SmartPointer
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
    using RGB16Pixel = itk::RGBPixel<uint16_t>;
    using RGB16Image = itk::Image<RGB16Pixel, Dimension>;
    using Mono16Image = itk::Image<uint16_t, Dimension>;

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
        << ": Error reading fixed image: '"
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
        << ": Exception object caught: '"
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
        << ": Exception object caught: '"
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
        << ": Exception object caught: '"
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
        << ": Exception object caught: '"
        << error
        << "\n";
      throw EXIT_FAILURE;
    }

    // Try to read the images
    /*
    try {
      fixed_reader->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << exec_name
        << ": Error reading image: '"
        << user_options.fixed_image
        << "'. '"
        << error
        << "\n";
      return EXIT_FAILURE;
    }
    try {
      moving_reader->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << exec_name
        << ": Error reading image: '"
        << user_options.moving_image
        << "'. '"
        << error
        << "\n";
      return EXIT_FAILURE;
    }
    */
    
    // The transform that will map the fixed image into the moving image.
    using TransformType = itk::AffineTransform<double, Dimension>;

    //  An optimizer is required to explore the parameter space of the transform
    //  in search of optimal values of the metric.
    using OptimizerType = itk::RegularStepGradientDescentOptimizer;

    //  The metric will compare how well the two images match each other. Metric
    //  types are usually parameterized by the image types as it can be seen in
    //  the following type declaration.
    using MetricType
      = itk::MeanSquaresImageToImageMetric<Mono16Image, Mono16Image>;

    //  Finally, the type of the interpolator is declared. The interpolator will
    //  evaluate the intensities of the moving image at non-grid positions.
    using InterpolatorType
      = itk::LinearInterpolateImageFunction<Mono16Image, double>;

    //  The registration method type is instantiated using the types of the
    //  fixed and moving images. This class is responsible for interconnecting
    //  all the components that we have described so far.
    using RegistrationType
      = itk::ImageRegistrationMethod<Mono16Image, Mono16Image>;

    // Create components
    auto metric = MetricType::New();
    auto transform = TransformType::New();
    auto optimizer = OptimizerType::New();
    auto interpolator = InterpolatorType::New();
    auto registration = RegistrationType::New();

    // Each component is now connected to the instance of the
    // registration method
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    registration->SetTransform(transform);
    registration->SetInterpolator(interpolator);

    // Set the registration inputs
    registration->SetFixedImage(fixed_luminance);
    registration->SetMovingImage(moving_luminance);

    registration->SetFixedImageRegion(
      fixed_luminance->GetLargestPossibleRegion()
      );

    //  Initialize the transform
    using ParametersType = RegistrationType::ParametersType;
    ParametersType initialParameters(transform->GetNumberOfParameters());

    // rotation matrix
    initialParameters[0] = 1.0; // R(0,0)
    initialParameters[1] = 0.0; // R(0,1)
    initialParameters[2] = 0.0; // R(1,0)
    initialParameters[3] = 1.0; // R(1,1)

    // translation vector
    initialParameters[4] = 0.0;
    initialParameters[5] = 0.0;

    registration->SetInitialTransformParameters(initialParameters);

    optimizer->SetMaximumStepLength(.1); // If this is set too high, you will
                                         // get a "itk::ERROR::
                                         // MeanSquaresImageToImageMetric
                                         // (0xa27ce70): Too many samples map
                                         // outside moving image buffer:
                                         // 1818 10000" error

    optimizer->SetMinimumStepLength(0.01);

    // Set a stopping criterion
    optimizer->SetNumberOfIterations(200);

    // Connect an observer
    auto observer = itk::CommandIterationUpdate<OptimizerType>::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    std::cout << "Starting registration\n";
    try {
      registration->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << exec_name
        << ": Error registering images: "
        << error << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "Registration done\n";

    // The result of the registration process is an array of parameters that
    // defines the spatial transformation in an unique way. This final result
    // is obtained using the GetLastTransformParameters() method.

    ParametersType finalParameters = registration->GetLastTransformParameters();
    std::cout << "Final parameters: " << finalParameters << "\n";

    // The value of the image metric corresponding to the last set of
    // parameters can be obtained with the GetValue() method of the optimizer.

    const double bestValue = optimizer->GetValue();

    // Print out results
    //
    std::cout << "Result = \n";
    std::cout << " Metric value  = " << bestValue << "\n";

    /*
    //  It is common, as the last step of a registration task, to use the
    //  resulting transform to map the moving image into the fixed image space.
    //  This is easily done with the ResampleImageFilter.

    using ResampleFilterType = itk::ResampleImageFilter<RGB16Image, RGB16Image>;

    auto resampler = ResampleFilterType::New();
    resampler->SetInput(moving_image);

    // The Transform that is produced as output of the Registration method is
    // also passed as input to the resampling filter. Note the use of the
    // methods GetOutput() and Get(). This combination is needed here because
    // the registration method acts as a filter whose output is a transform
    // decorated in the form of a DataObject. For details in this construction
    // you may want to read the documentation of the DataObjectDecorator.

    resampler->SetTransform(registration->GetOutput()->Get());

    // As described in Section sec:ResampleImageFilter, the ResampleImageFilter
    // requires additional parameters to be specified, in particular, the
    // spacing, origin and size of the output image. The default pixel value is
    // also set to a distinct gray level in order to highlight the regions that
    // are mapped outside of the moving image.

    resampler->SetSize(fixed_image->GetLargestPossibleRegion().GetSize());
    resampler->SetOutputOrigin(fixed_image->GetOrigin());
    resampler->SetOutputSpacing(fixed_image->GetSpacing());
    resampler->SetOutputDirection(fixed_image->GetDirection());
    RGB16Image::PixelType defaultPixelValue;
    defaultPixelValue[0] = std::numeric_limits<uint16_t>::max();
    defaultPixelValue[1] = std::numeric_limits<uint16_t>::max();
    defaultPixelValue[2] = std::numeric_limits<uint16_t>::max();
    resampler->SetDefaultPixelValue(defaultPixelValue);

    // The output of the filter is passed to a writer that will store the
    // image in a file. An CastImageFilter is used to convert the pixel type of
    // the resampled image to the final type used by the writer. The cast and
    // writer filters are instantiated below.

    using CastFilterType = itk::CastImageFilter<RGB16Image, RGB16Image>;

    auto caster = CastFilterType::New();
    caster->SetInput(resampler->GetOutput());

    auto writer = RGB16Writer::New();
    writer->SetFileName(out_base_name + "_registered" + out_extension);
    writer->SetInput(caster->GetOutput());

    // The complete pipeline is executed by invoking Update() on the writer.
    try {
      writer->Update();
    } catch (const itk::ExceptionObject & error) {
      std::cerr << exec_name
        << ": Exception object caught: '"
        << error
        << "\n";
      throw EXIT_FAILURE;
    }
    */

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

