// ============================================================================
// split_channels.cxx (ITK_Playground) - Split color channels of an image
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
// * split_channels.cxx: created.
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
#include <cctype>                    // required by std::tolower
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
#include <itkImage.h>                // required by itk::Image
#include <itkImageAdaptor.h>         // required by itk::ImageAdaptor
#include <itkImageFileReader.h>      // required for the reading image data
#include <itkImageFileWriter.h>      // required for writing image data to file
#include <itkRescaleIntensityImageFilter.h>  // required for rescaling image
                                             // intensities
#include <itkSmartPointer.h>         // required by itk::SmartPointer
#include <itkTIFFImageIO.h>          // required for reading and writing
                                     // TIFF images


// ============================================================================
// Global constants section
// ============================================================================

static const std::string kAppName = "split_channels";
static const std::string kVersionString = "0.1";
static const std::string kYearString = "2024";
static const std::string kAuthorName = "Ljubomir Kurij";
static const std::string kAuthorEmail = "ljubomir_kurij@protonmail.com";
static const std::string kAppDoc = "\
Split color channels of an image.\n\n\
Mandatory arguments to long options are mandatory for short options too.\n";
static const std::string kLicense = "\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";


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
std::string str_tolower(std::string);


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
    std::string channel;
    bool overwrite;
    std::vector<std::string> unsupported;
  };

  // Define the default values for the command line options
  CLIOptions user_options{
      false,        // show_help
      false,        // print_usage
      false,        // show_version
      "",           // input_file
      "all",        // channel
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
        clipp::option("-c", "--channel")
          .doc("color channel to extract (R, G, B, all) [default: all]")
        & clipp::opt_value(istarget, "CHANNEL", user_options.channel),
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

    // Check if the channel switch was triggered. If it was, we check if
    // the value is valid.
    std::string ch = std::string(str_tolower(user_options.channel));

    if (ch != "r"
        && ch != "red"
        && ch != "g"
        && ch != "green"
        && ch != "b"
        && ch != "blue"
        && ch != "all"
        ) {
      std::cerr << kAppName
        << ": Invalid color channel value: "
        << user_options.channel
        << "\n";

      // Print short help message
      printShortHelp(exec_name);

      throw EXIT_FAILURE;
    }

    user_options.channel = ch;

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
        ("r" == user_options.channel
          || "red" == user_options.channel
          || "all" == user_options.channel
          )
        && !user_options.overwrite
        && fs::exists (out_base_name + "_R" + out_extension)
        ) {
      std::cerr << kAppName
        << ": Output file already exists: "
        << out_base_name << "_R" << out_extension
        << "\n";
      throw EXIT_FAILURE;
    }
    if (
        ("g" == user_options.channel
          || "green" == user_options.channel
          || "all" == user_options.channel
          )
        && !user_options.overwrite
        && fs::exists (out_base_name + "_G" + out_extension)
        ) {
      std::cerr << kAppName
        << ": Output file already exists: "
        << out_base_name << "_G" << out_extension
        << "\n";
      throw EXIT_FAILURE;
    }
    if (
        ("b" == user_options.channel
          || "blue" == user_options.channel
          || "all" == user_options.channel
          )
        && !user_options.overwrite
        && fs::exists (out_base_name + "_B" + out_extension)
        ) {
      std::cerr << kAppName
        << ": Output file already exists: "
        << out_base_name << "_B" << out_extension
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
    if (tiffImageIO->GetCompressor() != "NoCompression"
      && !tiffImageIO->GetCompressor().empty()
    ) {
      std::cerr << kAppName
        << ": File is compressed: "
        << user_options.input_file
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if we are dealing with the RGB image
    if (tiffImageIO->GetNumberOfComponents() != 3) {
      std::cerr << kAppName
        << ": File is not an RGB image: "
        << user_options.input_file
        << "\n";
      throw EXIT_FAILURE;
    }

    // Check if we are dealing with 16-bit image. GetComponentSize() returns
    // bytes, so 2 bytes = 16 bits.
    if (tiffImageIO->GetComponentSize() != 2) {
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

    // Define the adaptors for the color channels
    using RedChannelAdaptor = itk::ImageAdaptor<RGB16Image,
        RGB16ColorChannelAccessor<ColorChannel::R>>;
    using GreenChannelAdaptor = itk::ImageAdaptor<RGB16Image,
        RGB16ColorChannelAccessor<ColorChannel::G>>;
    using BlueChannelAdaptor = itk::ImageAdaptor<RGB16Image,
        RGB16ColorChannelAccessor<ColorChannel::B>>;
    using RedChannelRescalerType
      = itk::RescaleIntensityImageFilter<RedChannelAdaptor, Mono16Image>;
    using GreenChannelRescalerType
      = itk::RescaleIntensityImageFilter<GreenChannelAdaptor, Mono16Image>;
    using BlueChannelRescalerType
      = itk::RescaleIntensityImageFilter<BlueChannelAdaptor, Mono16Image>;
    
    // Instantiate objects and connect them
    auto reader = RGB16Reader::New();
    reader->SetFileName(user_options.input_file);
    auto red_channel = RedChannelAdaptor::New();
    red_channel->SetImage(reader->GetOutput());
    auto green_channel = GreenChannelAdaptor::New();
    green_channel->SetImage(reader->GetOutput());
    auto blue_channel = BlueChannelAdaptor::New();
    blue_channel->SetImage(reader->GetOutput());
    auto red_rescaler = RedChannelRescalerType::New();
    red_rescaler->SetOutputMinimum(std::numeric_limits<uint16_t>::min());
    red_rescaler->SetOutputMaximum(std::numeric_limits<uint16_t>::max());
    red_rescaler->SetInput(red_channel);
    auto green_rescaler = GreenChannelRescalerType::New();
    green_rescaler->SetOutputMinimum(std::numeric_limits<uint16_t>::min());
    green_rescaler->SetOutputMaximum(std::numeric_limits<uint16_t>::max());
    green_rescaler->SetInput(green_channel);
    auto blue_rescaler = BlueChannelRescalerType::New();
    blue_rescaler->SetOutputMinimum(std::numeric_limits<uint16_t>::min());
    blue_rescaler->SetOutputMaximum(std::numeric_limits<uint16_t>::max());
    blue_rescaler->SetInput(blue_channel);
    auto red_writer = Mono16Writer::New();
    red_writer->SetFileName((out_base_name + "_R" + out_extension).c_str());
    red_writer->SetInput(red_rescaler->GetOutput());
    auto green_writer = Mono16Writer::New();
    green_writer->SetFileName((out_base_name + "_G" + out_extension).c_str());
    green_writer->SetInput(green_rescaler->GetOutput());
    auto blue_writer = Mono16Writer::New();
    blue_writer->SetFileName((out_base_name + "_B" + out_extension).c_str());
    blue_writer->SetInput(blue_rescaler->GetOutput());


    // Write channels to files
    if (
        "r" == user_options.channel
        || "red" == user_options.channel
        || "all" == user_options.channel
        ) {
      try {
        red_writer->Update();
      } catch (const itk::ExceptionObject & error) {
        std::cerr << kAppName
          << ": Error writing file: '"
          << out_base_name << "_R" << out_extension
          << "'. "
          << error
          << "\n";
        throw EXIT_FAILURE;
      }
    }
    if (
        "g" == user_options.channel
        || "green" == user_options.channel
        || "all" == user_options.channel
        ) {
      try {
        green_writer->Update();
      } catch (const itk::ExceptionObject & error) {
        std::cerr << kAppName
          << ": Error writing file: '"
          << out_base_name << "_G" << out_extension
          << "'. "
          << error
          << "\n";
        throw EXIT_FAILURE;
      }
    }
    if (
        "b" == user_options.channel
        || "blue" == user_options.channel
        || "all" == user_options.channel
        ) {
      try {
        blue_writer->Update();
      } catch (const itk::ExceptionObject & error) {
        std::cerr << kAppName
          << ": Error writing file: '"
          << out_base_name << "_B" << out_extension
          << "'. "
          << error
          << "\n";
        throw EXIT_FAILURE;
      }
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

std::string str_tolower(std::string s) {
  std::transform(
    s.begin(),
    s.end(),
    s.begin(),
    [](unsigned char c){ return std::tolower(c); }
    );

  return s;
}