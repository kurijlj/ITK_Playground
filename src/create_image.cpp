// ============================================================================
// <one line to give the program's name and a brief idea of what it does.>
//
//  Copyright (C) <yyyy> <Author Name> <author@mail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// ============================================================================

// ============================================================================
//
// <Put documentation here>
//
// <yyyy>-<mm>-<dd> <Author Name> <author@mail.com>
//
// * <programfilename>.cpp: created.
//
// ============================================================================

// ============================================================================
//
// References (this section should be deleted in the release version)
//
// * For coding style visit Google C++ Style Guide page at
//   <https://google.github.io/styleguide/cppguide.html>.
//
// * For command line arguments parsing using clipp consult documentation and
//   examples at <https://github.com/muellan/clipp>.
//
// * For filesystem operations (C++17) visit 'filesystem' reference at:
//   <https://en.cppreference.com/w/cpp/filesystem>.
//
// * For SQLite3 C/C++ interface documentation visit
//   <https://www.sqlite.org/c3ref/intro.html>.
//
// * For libsodium library documentation visit
//   <https://libsodium.gitbook.io/doc/>.
//
// * For CSV parser library documentation visit
//   <https://github.com/vincentlaucsb/csv-parser>
//
// ============================================================================

// ============================================================================
// Preprocessor directives section
// ============================================================================
#define SODIUM_STATIC                                                          \
  1 // Required for 'sodium.h' to work properly if using
    // static linking of the library

// ============================================================================
// Headers include section
// ============================================================================

// Related header

// "C" system headers

// Standard Library headers
#include <cstdlib>    // required by EXIT_SUCCESS, EXIT_FAILURE
#include <exception>  // required by std::current_exception
#include <filesystem> // required by std::filesystem
#include <iostream>   // required by cin, cout, ...
#include <string>     // required by std::string

// External libraries headers
#include <clipp.hpp> // command line arguments parsing
#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkTIFFImageIO.h>

// ============================================================================
// Namespace alias section
// ============================================================================

// namespace fs = std::filesystem;

// ============================================================================
// Global constants section
// ============================================================================

static const std::string kAppName = "create_image";
static const std::string kVersionString = "0.1";
static const std::string kYearString = "2024";
static const std::string kAuthorName = "Ljubomir Kurij";
static const std::string kAuthorEmail = "ljubomir_kurij@protonmail.com";
static const std::string kAppDoc = "\
A simple test of ITK create image and write to image facilities.\n\n\
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
    std::vector<std::string> unsupported;
  };

  // Define the default values for the command line options
  CLIOptions user_options{
      false, // show_help
      false, // print_usage
      false, // show_version
      {}     // unsupported options aggregator
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
      (clipp::option("-h", "--help")
           .set(user_options.show_help)
           .doc("show this help message and exit"),
       clipp::option("--usage")
           .set(user_options.print_usage)
           .doc("give a short usage message"),
       clipp::option("-V", "--version")
           .set(user_options.show_version)
           .doc("print program version"))
          .doc("general options:"),
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

    // Main code goes here ----------------------------------------------------
    using ImageType = itk::Image<unsigned short int, 2>; // 2D image with
                                                         // unsigned short
                                                         // int pixels
    auto image = ImageType::New();

    ImageType::RegionType region;
    ImageType::IndexType start;
    start[0] = 0;
    start[1] = 0;
    ImageType::SpacingType spacing;
    spacing[0] = 25.4 / 400.0;
    spacing[1] = 25.4 / 400.0;

    ImageType::SizeType size;
    unsigned int NumRows = 400;
    unsigned int NumCols = 400;
    size[0] = NumRows;
    size[1] = NumCols;

    region.SetSize(size);
    region.SetIndex(start);

    image->SetRegions(region);
    image->SetSpacing(spacing);
    image->Allocate();
    image->FillBuffer(0);

    // Make a square
    unsigned int squareOrigin[2] = {50, 50};
    unsigned int squareSize[2] = {100, 100};
    for (unsigned int x = squareOrigin[0]; x <= squareOrigin[0] + squareSize[0];
         ++x) {
      for (unsigned int y = squareOrigin[1];
           y < squareOrigin[1] + squareSize[1]; ++y) {
        ImageType::IndexType pixelIndex;
        pixelIndex[0] = x;
        pixelIndex[1] = y;

        image->SetPixel(pixelIndex, 65535);
      }
    }

    using WriterType = itk::ImageFileWriter<ImageType>;
    using TIFFIOType = itk::TIFFImageIO;

    auto tiffIO = TIFFIOType::New();
    tiffIO->SetPixelType(itk::IOPixelEnum::RGB);

    auto writer = WriterType::New();
    writer->SetFileName("output.tiff");
    writer->SetInput(image);
    writer->SetImageIO(tiffIO);

    try {
      writer->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << "Error: " << error << std::endl;

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
