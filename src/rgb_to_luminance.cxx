// ============================================================================
// rgb_to_luminance.cxx (ITK_Playground) - Convert RGB image to luminance image
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
// 2024-07-22 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
//
// * rgb_to_luminance.cxx: created.
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
#include <string>                    // required by std::string
#include <string_view>               // required by std::string_view
#include <vector>                    // required by std::vector

// External libraries headers
#include <clipp.hpp>                 // command line arguments parsing
#include <itkImage.h>                // required by itk::Image
#include <itkImageFileReader.h>      // required for the reading image data
#include <itkImageFileWriter.h>      // required for writing image data to file
#include <itkRGBToLuminanceImageFilter.h>  // required for converting RGB to
                                           // luminance image
#include <itkSmartPointer.h>         // required by itk::SmartPointer
#include <itkTIFFImageIO.h>          // required for reading and writing
                                     // TIFF images


// ============================================================================
// Global constants section
// ============================================================================

static constexpr auto kAppName = "rgb_to_luminance";
static constexpr auto kVersionString = "0.1";
static constexpr auto kYearString = "2024";
static constexpr auto kAuthorName = "Ljubomir Kurij";
static constexpr auto kAuthorEmail = "ljubomir_kurij@protonmail.com";
static constexpr auto kAppDoc = "\
Convert RGB image to luminance image.\n\n\
Mandatory arguments to long options are mandatory for short options too.\n";
static constexpr auto kLicense = "\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";


// ============================================================================
// Global variables section
// ============================================================================

static std::string_view exec_name = kAppName;


// ============================================================================
// Utility function prototypes
// ============================================================================

void printShortHelp(const std::string_view & = kAppName);
void printUsage(
  const clipp::group &,
  const std::string_view & = kAppName,
  const clipp::doc_formatting & = clipp::doc_formatting{}
  );
void printVersionInfo(
  const std::string_view & = kAppName,
  const std::string_view & = kVersionString,
  const std::string_view & = kYearString,
  const std::string_view & = kAuthorName,
  const std::string_view & = kLicense
  );
void showHelp(
  const clipp::group &,
  const std::string_view & = kAppName,
  const std::string_view & = kAppDoc,
  const std::string_view & = kAuthorEmail
  );


// ============================================================================
// Utility class definitions
// ============================================================================

// Define the color channels
enum ColorChannel { R, G, B };

// Define the pixel type for the RGB image with 16-bit unsigned integer values
using RGB16Pixel = itk::RGBPixel<uint16_t>;  // RGB pixel with 16-bit
                                             // unsigned integer values

// Define the accessor class for accessing the color channels of the RGB16Pixel
template <ColorChannel channel>
class RGB16ColorChannelAccessor {
public:
  using InternalType = RGB16Pixel;
  using ExternalType = uint16_t;

  static ExternalType
  Get(const InternalType &input) {
    switch (channel) {
      case ColorChannel::R:
        return input.GetRed();
      case ColorChannel::G:
        return input.GetGreen();
      case ColorChannel::B:
        return input.GetBlue();
      default:
        return 0;  // Should never reach this point
    }
  }
};


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
    bool overwrite;
    std::vector<std::string> unsupported;
  };

  // Define the default values for the command line options
  CLIOptions user_options{
      false,        // show_help
      false,        // print_usage
      false,        // show_version
      "",           // input_file
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
        clipp::opt_value(istarget, "INPUT_FILE", user_options.input_file),
        clipp::option("-o", "--overwrite")
          .set(user_options.overwrite)
          .doc("overwrite existing files"),
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

      std::cout << "\n";

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
        << "\n";
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

    // Decompose the input file name into the base name and the extension
    std::string out_base_name
      = fs::path(user_options.input_file).stem().string();
    std::string out_extension
      = fs::path(user_options.input_file).extension().string();

    // Check if the output file(s) already exists
    if (
        !user_options.overwrite
        && fs::exists (out_base_name + "_luminance" + out_extension)
        ) {
      std::cerr << kAppName
        << ": Output file already exists: "
        << out_base_name << "_luminance" << out_extension
        << "\n";
      throw EXIT_FAILURE;
    }

    // Main code goes here ----------------------------------------------------
    // Instantiate the TIFF image reader
    auto tiffImageIO = itk::TIFFImageIO::New();

    // Check if we are dealing with a regular TIFF image
    if (!tiffImageIO->CanReadFile(user_options.input_file.c_str())) {
      std::cerr << kAppName
        << ": File is not a regular TIFF image: "
        << user_options.input_file
        << "\n";
      throw EXIT_FAILURE;
    }

    // Set the file name and read the image information
    tiffImageIO->SetFileName(user_options.input_file);
    tiffImageIO->ReadImageInformation();

    // Check if we are dealing with an compressed image
    if (tiffImageIO->ReadCompressionFromImage() != 1) {  // 1 = no compression
      std::cerr << kAppName
        << ": File is compressed: "
        << user_options.input_file
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if we are dealing with the RGB image
    if (tiffImageIO->ReadSamplesPerPixelFromImage() != 3) {
      std::cerr << kAppName
        << ": File is not an RGB image: "
        << user_options.input_file
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if we are dealing with 16-bit image
    if (tiffImageIO->ReadBitsPerSampleFromImage() != 16) {
      std::cerr << kAppName
        << ": File is not a 16-bit image: "
        << user_options.input_file
        << "\n";
      throw EXIT_FAILURE;
    }

    // Define accessor and utility classes for accessing the color channels
    // Define image types for the input and output images
    using RGB16Image = itk::Image<RGB16Pixel, 2>;
    using Mono16Image = itk::Image<uint16_t, 2>;

    // Define the image reader and writer types
    using RGB16Reader = itk::ImageFileReader<RGB16Image>;
    using Mono16Writer = itk::ImageFileWriter<Mono16Image>;

    // Define the luminance filter type
    using LuminanceFilterType
      = itk::RGBToLuminanceImageFilter<RGB16Image, Mono16Image>;

    // Instantiate objects and connect them
    auto reader = RGB16Reader::New();
    reader->SetFileName(user_options.input_file);
    auto filter = LuminanceFilterType::New();
    filter->SetInput(reader->GetOutput());
    auto writer = Mono16Writer::New();
    writer->SetFileName((out_base_name + "_luminance" + out_extension).c_str());
    writer->SetInput(filter->GetOutput());

    // Write luminance image to file
    try {
      writer->Update();
    } catch (const itk::ExceptionObject & error) {
      std::cerr << kAppName
        << ": Error writing file: '"
        << out_base_name << "_luminance" << out_extension
        << "'. "
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

inline void printShortHelp(const std::string_view &exec_name) {
  std::cout << "Try '" << exec_name << " --help' for more information.\n";
}

inline void printUsage(
    const clipp::group &group,
    const std::string_view &prefix,
    const clipp::doc_formatting &fmt
    ) {
  std::cout
    << clipp::usage_lines(group, static_cast<std::string>(prefix), fmt)
    << "\n";
}

void printVersionInfo(
    const std::string_view &exec_name,
    const std::string_view &app_version,
    const std::string_view &release_year,
    const std::string_view &author_name,
    const std::string_view &license
) {
  std::cout << kAppName << " " << kVersionString << " Copyright (C) "
            << kYearString << " " << kAuthorName << "\n"
            << kLicense;
}

void showHelp(
    const clipp::group &group,
    const std::string_view &exec_name,
    const std::string_view &doc,
    const std::string_view &author_email
    ) {
  auto fmt = clipp::doc_formatting{}.first_column(0).last_column(79);
  clipp::man_page man;

  man.prepend_section("USAGE",
    clipp::usage_lines(group, static_cast<std::string>(exec_name), fmt).str());
  man.append_section("", static_cast<std::string>(doc));
  man.append_section("", clipp::documentation(group, fmt).str());
  man.append_section("",
    "Report bugs to <" + static_cast<std::string>(author_email) + ">.");

  std::cout << man;
}
