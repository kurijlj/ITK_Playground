// ============================================================================
// create_image_from_buffer.cxx (ITK_Playground) - Create image from buffer
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
// 2024-08-15 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
//
// * create_image_from_buffer.cxx: created.
//
// ============================================================================


// ============================================================================
// Preprocessor directives section
// ============================================================================


// ============================================================================
// Headers include section
// ============================================================================

// Related header

// "C" system headers

// Standard Library headers
#include <cmath>        // required by std::ceil, std::floor, std::round, ...
#include <cstdlib>      // required by EXIT_SUCCESS, EXIT_FAILURE
#include <exception>    // required by std::current_exception
#include <filesystem>   // required by std::filesystem
#include <iostream>     // required by cin, cout, ...
#include <limits>       // required by std::numeric_limits
#include <memory>       // required by std::unique_ptr
#include <string>       // required by std::string
#include <type_traits>  // required by std::is_integer, ...

// External libraries headers
#include <clipp.hpp> // command line arguments parsing
#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkTIFFImageIO.h>


// ============================================================================
// Global constants section
// ============================================================================

static constexpr auto kAppName = "create_image_from_buffer";
static constexpr auto kVersionString = "0.1";
static constexpr auto kYearString = "2024";
static constexpr auto kAuthorName = "Ljubomir Kurij";
static constexpr auto kAuthorEmail = "ljubomir_kurij@protonmail.com";
static constexpr auto kAppDoc = "\
Create image from buffer and write it to a file.\n\n\
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
// User defined template functions
// ============================================================================
// std::unique_ptr<Base> b2(new Base);
// Derived * p = static_cast<Derived *>(b2.get());

template <typename PixelType, typename SizeType>
std::unique_ptr<PixelType[]> create_raw_buffer_image(
    SizeType width, SizeType height) {
  static_assert(std::numeric_limits<PixelType>::is_integer
    || std::is_floating_point<PixelType>::value,
    "'PixelType' must be integer or floating point!"
    );
  static_assert(std::numeric_limits<SizeType>::is_integer
    && !std::numeric_limits<SizeType>::is_signed,
    "'SizeType' must be unsigned integer!"
    );
  
  std::unique_ptr<PixelType[]> buffer(new PixelType[width * height]{0});

  for (auto j = 0; j < height; ++j) {
    for (auto i = 0; i < width; ++i) {
      auto slope     = static_cast<double>(height - 1)
                      / static_cast<double>(width - 1);
      auto intersec  = static_cast<double>(height - 1);
      auto x         = static_cast<double>(i);
      SizeType limit = 0;
      
      if (width < height) {
        limit = static_cast<SizeType>(std::ceil(intersec - slope * x));
      } else {
        limit = static_cast<SizeType>(std::round(intersec - slope * x));
      }

      if (j <= limit) {
        buffer[j*width + i] = std::numeric_limits<PixelType>::max();
      } else {
        buffer[j*width + i] = std::numeric_limits<PixelType>::min();
      }
    }
  }
  return buffer;
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
    writer->SetFileName("output.tif");
    writer->SetInput(image);
    writer->SetImageIO(tiffIO);

    try {
      writer->Update();
    } catch (const itk::ExceptionObject &error) {
      std::cerr << exec_name
        << ": Error writing file: 'output.tif'. "
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
      std::cerr << exec_name << ": Unhandled exception: " << e.what()
                << std::endl;
    }

    // Return an error code
    return EXIT_FAILURE;
  }

  // The code should never reach this point. If it does, print an error
  // message and exit
  std::cerr << exec_name << ": Unhandled program exit!" << std::endl;

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
