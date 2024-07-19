// ============================================================================
// image_affine_transform.cpp (ITK_Playground) - Rotate and translate an image
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
// 2024-07-18 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
//
// * image_affine_transform.cpp: created.
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
#include <cmath>                     // required by std::cos, std::sin
#include <cstdlib>                   // required by EXIT_SUCCESS, EXIT_FAILURE

// Standard Library headers
#include <exception>                 // required by std::current_exception
#include <filesystem>                // required by std::filesystem
#include <iostream>                  // required by cin, cout, ...
#include <string>                    // required by std::string

// External libraries headers
#include <clipp.hpp>                 // command line arguments parsing
#include <itkAffineTransform.h>      // required for affine transformation
#include <itkImage.h>                // required by itk::Image
#include <itkImageFileReader.h>      // required for the reading image data
#include <itkImageFileWriter.h>      // required for writing image data to file
#include <itkResampleImageFilter.h>  // required for resampling the image
#include <itkSmartPointer.h>         // required for smart pointers
#include <itkTIFFImageIO.h>          // required for reading and writing
                                     // TIFF images
#include <itkWindowedSincInterpolateImageFunction.h>  // required for
                                                      // interpolating the image


// ============================================================================
// Global constants section
// ============================================================================

static const std::string kAppName = "image_affine_transform";
static const std::string kVersionString = "0.1";
static const std::string kYearString = "2024";
static const std::string kAuthorName = "Ljubomir Kurij";
static const std::string kAuthorEmail = "ljubomir_kurij@protonmail.com";
static const std::string kAppDoc = "\
Rotate and translate an image using ITK.\n\n\
Mandatory arguments to long options are mandatory for short options too.\n";
static const std::string kLicense = "\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";

static const double kPi = 3.14159265358979323846;


// ============================================================================
// Global variables section
// ============================================================================

static std::string exec_name = kAppName;


// ============================================================================
// Utility function prototypes
// ============================================================================

void printShortHelp(std::string = kAppName);
void printUsage(const clipp::group &, const std::string = kAppName,
                const clipp::doc_formatting & = clipp::doc_formatting{});
void printVersionInfo();
void showHelp(const clipp::group &, const std::string = kAppName,
              const std::string = kAppDoc);


// ============================================================================
// Main Function Section
// ============================================================================

int main(int argc, char *argv[]) {
  namespace fs = std::filesystem; // Filesystem alias

  // Determine the exec name under wich program is beeing executed
  fs::path exec_path{argv[0]};
  exec_name = exec_path.filename().string();

  // Here we define the structure for holding the passed command line otions.
  // The structure is also used to define the command line options and their
  // default values.
  struct CLIOptions {
    bool show_help;
    bool print_usage;
    bool show_version;
    std::string input_file;
    std::string output_file;
    std::vector<std::string> unsupported;
  };

  // Define the default values for the command line options
  CLIOptions user_options{
      false,        // show_help
      false,        // print_usage
      false,        // show_version
      "",           // input_file
      "result.tif", // output_file
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
        clipp::opt_value(istarget, "INPUT_FILE", user_options.input_file),
        clipp::opt_value(istarget, "OUTPUT_FILE", user_options.output_file),
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
      std::cerr << kAppName << ": Unsupported options: ";
      for (const auto &opt : user_options.unsupported) {
        std::cerr << opt << " ";
      }
      std::cerr << std::endl;
      printShortHelp(exec_name);

      throw EXIT_FAILURE;
    }

    // Check if the help switch was triggered. We give help switch the
    // highest priority, so if it is triggered we don't need to check
    // anything else.
    if (user_options.show_help) {
      showHelp(parser_config, exec_name);

      throw EXIT_SUCCESS;
    }

    // Check if the usage switch was triggered. Usge switch has the second
    // highest priority, so if it is triggered we don't need to check
    // anything else.
    if (user_options.print_usage) {
      auto fmt = clipp::doc_formatting{}.first_column(0).last_column(79);
      printUsage(parser_config, exec_name, fmt);

      throw EXIT_SUCCESS;
    }

    // Check if the version switch was triggered. Version switch has the
    // third highest priority.
    if (user_options.show_version) {
      printVersionInfo();

      throw EXIT_SUCCESS;
    }

    // No high priority switch was triggered. Now we check if the input
    // file was passed. If not we print the usage message and exit.
    if (user_options.input_file.empty()) {
      auto fmt = clipp::doc_formatting {}
        .first_column(0)
        .last_column(79)
        .merge_alternative_flags_with_common_prefix(true);
      std::cout << "Usage: ";
      printUsage(parser_config, exec_name, fmt);

      std::cout << std::endl;

      // Print short help message
      printShortHelp(exec_name);

      throw EXIT_FAILURE;
    }

    // Input file was passed. Now we check if the file exists, is
    // readable and is a regular file and not an empty file.
    // Check if the file exists
    if (!fs::exists (user_options.input_file)) {
      std::cerr << kAppName
        << ": File does not exist: "
        << user_options.input_file
        << std::endl;
      throw EXIT_FAILURE;
    }

    // Check if the file is a regular file
    if (!fs::is_regular_file (user_options.input_file)) {
      std::cerr << kAppName
        << ": Not a regular file: "
        << user_options.input_file
        << std::endl;
      throw EXIT_FAILURE;
    }

    // Check if the file is empty
    if (fs::file_size (user_options.input_file) == 0) {
      std::cerr << kAppName
        << ": Empty file: "
        << user_options.input_file
        << std::endl;
      throw EXIT_FAILURE;
    }

    // Open the file in binary mode for wider compatibility
    std::ifstream file (
      user_options.input_file,
      std::ios::binary
      );

    // Check if the file was opened successfully
    // (if we can read it)
    if (!file.is_open()) {
      std::cerr << kAppName
        << ": Error opening file: "
        << user_options.input_file
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if the output file already exists
    if (fs::exists (user_options.output_file)) {
      std::cerr << kAppName
        << ": Output file already exists: "
        << user_options.output_file
        << "\n";
      throw EXIT_FAILURE;
    }

    // Main code goes here ----------------------------------------------------
    constexpr unsigned int Dimension = 2;  // We are working with 2D images
    constexpr unsigned int Radius = 3;   // Radius of the interpolation window
    using ScalarType = double;  // We are using double precision floating point
                                // values for the affine transformation matrix
    // Define the affine transformation matrix type
    using MatrixType = itk::Matrix<ScalarType, Dimension + 1, Dimension + 1>;  
    using RGB16Pixel = itk::RGBPixel<uint16_t>;  // RGB pixel with 16-bit
                                                 // unsigned integer values
    RGB16Pixel defaultFillValue;
    defaultFillValue[0] = 0;  // Default pixel value for the image
    defaultFillValue[1] = 0;
    defaultFillValue[2] = 0;
    using RGB16Image = itk::Image<RGB16Pixel, 2>;   // 2D RGB image with 16-bit
                                                    // unsigned integer pixel
                                                    // values
    // Define the image resampling filter type
    using ResampleImageFilterType
      = itk::ResampleImageFilter<RGB16Image, RGB16Image>;
    // Use the windowed sinc function to interpolate (minimize aliasing)
    using InterpolatorType =
      itk::WindowedSincInterpolateImageFunction<RGB16Image, Radius>;
    // Define the affine transformation type
    using TransformType = itk::AffineTransform<ScalarType, Dimension>;
    using WriterType = itk::ImageFileWriter<RGB16Image>;
    using TIFFIOType = itk::TIFFImageIO;

    // Read the image from the file
    itk::SmartPointer<RGB16Image> input;

    try {
      input = itk::ReadImage<RGB16Image>(user_options.input_file);
    } catch (const itk::ExceptionObject & error) {
      std::cerr << kAppName
        << ": Error opening file: "
        << user_options.input_file
        << ". "
        << error
        << "\n";
      std::cerr << "Error: " << error << std::endl;
      throw EXIT_FAILURE;
    }

    const RGB16Image::SizeType & size
      = input->GetLargestPossibleRegion().GetSize();

    auto resample = ResampleImageFilterType::New();
    resample->SetInput(input);
    resample->SetReferenceImage(input);
    resample->UseReferenceImageOn();
    resample->SetSize(size);
    resample->SetDefaultPixelValue(defaultFillValue);

    using InterpolatorType
      = itk::WindowedSincInterpolateImageFunction<RGB16Image, Radius>;
    auto interpolator = InterpolatorType::New();

    resample->SetInterpolator(interpolator);

    auto transform = TransformType::New();

    // Define the affine transformation matrix
    MatrixType matrix;
    matrix[0][0] = std::cos(kPi / 4.0);
    matrix[0][1] = std::sin(kPi / 4.0);
    matrix[0][2] = 0.;

    matrix[1][0] = -matrix[0][1];
    matrix[1][1] = matrix[0][0];
    matrix[1][2] = 0.;

    matrix[2][0] = -100.;
    matrix[2][1] = -100.;
    matrix[2][2] = 1.;   

    // get transform parameters from MatrixType
    TransformType::ParametersType parameters(Dimension * Dimension + Dimension);
    for (unsigned int i = 0; i < Dimension; ++i)
    {
      for (unsigned int j = 0; j < Dimension; ++j)
      {
        parameters[i * Dimension + j] = matrix[i][j];
      }
    }
    for (unsigned int i = 0; i < Dimension; ++i)
    {
      parameters[i + Dimension * Dimension] = matrix[i][Dimension];
    }
    transform->SetParameters(parameters);

    resample->SetTransform(transform);

    // Write the image to the file
    try {
      auto tiffIO = TIFFIOType::New();
      tiffIO->SetPixelType(itk::IOPixelEnum::RGB);
      auto writer = WriterType::New();
      writer->SetFileName(user_options.output_file);
      writer->SetInput(resample->GetOutput());
      writer->SetImageIO(tiffIO);
      writer->Update();
      /*
      itk::WriteImage<RGB16Image>(
        resample->GetOutput(),
        user_options.output_file
        );
      */
    } catch (const itk::ExceptionObject & error) {
      std::cerr << kAppName
        << ": Error writing file: "
        << user_options.output_file
        << ". "
        << error
        << "\n";
      throw EXIT_FAILURE;
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
      std::cerr << kAppName << ": Unhandled exception: " << e.what()
                << std::endl;
    }

    // Return an error code
    return EXIT_FAILURE;
  }

  // The code should never reach this point. If it does, print an error
  // message and exit
  std::cerr << kAppName << ": Unhandled program exit!" << std::endl;

  return EXIT_FAILURE;
}


// ============================================================================
// Function definitions
// ============================================================================

inline void printShortHelp(std::string exec_name) {
  std::cout << "Try '" << exec_name << " --help' for more information.\n";
}

inline void printUsage(const clipp::group &group, const std::string prefix,
                       const clipp::doc_formatting &fmt) {
  std::cout << clipp::usage_lines(group, prefix, fmt) << "\n";
}

void printVersionInfo() {
  std::cout << kAppName << " " << kVersionString << " Copyright (C) "
            << kYearString << " " << kAuthorName << "\n"
            << kLicense;
}

void showHelp(const clipp::group &group, const std::string exec_name,
              const std::string doc) {
  auto fmt = clipp::doc_formatting{}.first_column(0).last_column(79);
  clipp::man_page man;

  man.prepend_section("USAGE", clipp::usage_lines(group, exec_name, fmt).str());
  man.append_section("", doc);
  man.append_section("", clipp::documentation(group, fmt).str());
  man.append_section("", "Report bugs to <" + kAuthorEmail + ">.");

  std::cout << man;
}
