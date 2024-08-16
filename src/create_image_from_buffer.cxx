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
#include <itkImage.h>              // required by itk::Image
#include <itkImageFileWriter.h>    // required by itk::ImageFileWriter
#include <itkImportImageFilter.h>  // required by itk::ImportImageFilter
#include <itkTIFFImageIO.h>        // required by itk::TIFFImageIO


// ============================================================================
// Global type aliases section
// ============================================================================

using ushort = unsigned short int;


// ============================================================================
// Global constants section
// ============================================================================

static constexpr auto kAppName       = "create_image_from_buffer";
static constexpr auto kVersionString = "0.1";
static constexpr auto kYearString    = "2024";
static constexpr auto kAuthorName    = "Ljubomir Kurij";
static constexpr auto kAuthorEmail   = "ljubomir_kurij@protonmail.com";
static constexpr auto kAppDoc        = "\
Create image from buffer and write it to a file.\n\n\
Mandatory arguments to long options are mandatory for short options too.\n";
static constexpr auto kLicense       = "\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";

// ============================================================================
// Global variables section
// ============================================================================


// ============================================================================
// Utility function prototypes
// ============================================================================

// ----------------------------------------------------------------------------
// 'printShortHelp' function
// ----------------------------------------------------------------------------
//
// Description:
// This function prints a short help message to the standard output. The
// message is intended to be used when the user passes wrong command line
// options.
//
// Parameters:
//   exec_name: The name of the executable file running the program.
//
// Returns:
// This function does not return a value.
//
// Exceptions:
// This function does not throw exceptions.
//
// ----------------------------------------------------------------------------
void printShortHelp (const char * = kAppName) noexcept;

// ----------------------------------------------------------------------------
// 'printUsage' function
// ----------------------------------------------------------------------------
//
// Description:
// This function prints the usage message to the standard output. The usage
// message is generated by the 'clipp' library and it is based on the command
// line options defined in the 'parser_config' variable.
//
// Parameters:
//   group: The 'clipp' group object that holds the command line options.
//   prefix: The name of the executable file running the program.
//   fmt: The 'clipp' formatting object that defines the formatting of the
//        usage message.
//
// Returns:
// This function does not return a value.
//
// Exceptions:
// See the 'clipp' library documentation for the exceptions that can be thrown
// by the 'usage_lines' function.
//
// ----------------------------------------------------------------------------
void printUsage (
  const clipp::group &,
  const char * = kAppName,
  const clipp::doc_formatting & = clipp::doc_formatting{}
  );

// ----------------------------------------------------------------------------
// 'printVersionInfo' function
// ----------------------------------------------------------------------------
//
// Description:
// This function prints the version information to the standard output. The
// version information includes the name of the program, the version string,
// the year of the release, the name of the author and the license information.
//
// Parameters:
//   exec_name: The name of the executable file running the program.
//   app_version: The version string of the program.
//   release_year: The year of the release.
//   author_name: The name of the author.
//   license: The license information.
//
// Returns:
// This function does not return a value.
//
// Exceptions:
// This function does not throw exceptions.
//
// ----------------------------------------------------------------------------
void printVersionInfo (
  const char * = kAppName,
  const char * = kVersionString,
  const char * = kYearString,
  const char * = kAuthorName,
  const char * = kLicense
  ) noexcept;

// ----------------------------------------------------------------------------
// 'showHelp' function
// ----------------------------------------------------------------------------
//
// Description:
// This function prints the help message to the standard output. The help
// message is generated by the 'clipp' library and it is based on the command
// line options defined in the 'parser_config' variable.
//
// Parameters:
//   group: The 'clipp' group object that holds the command line options.
//   exec_name: The name of the executable file running the program.
//   doc: The documentation string that describes the program.
//   author_email: The email address of the author.
//
// Returns:
// This function does not return a value.
//
// Exceptions:
// See the 'clipp' library documentation for the exceptions that can be thrown
// by the 'clipp::doc_formatting', 'clipp::usage_lines' and
// 'clipp::documentation' functions.
//
// ----------------------------------------------------------------------------
void showHelp (
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

// ----------------------------------------------------------------------------
// 'createBufferImage' template function
// ----------------------------------------------------------------------------
//
// Description:
// This function creates a buffer image with the specified width, height and
// pixel type. The buffer image is created as a 1D image.
//
// Created image is filled with the minimum and maximum values of the pixel
// type. The resulting image is diagonal with the minimum values in the upper
// left corner and the maximum values in the lower right corner.
//
// The buffer image is returned as a unique pointer to the buffer.
//
// Template parameters:
//   PixelType: The type of the pixel values in the buffer image.
//   SizeType: The type of the size values in the buffer image.
//
// Parameters:
//   width: The width of the buffer image.
//   height: The height of the buffer image.
//
// Returns:
// This function returns a unique pointer to the buffer image.
//
// Exceptions:
// This function does not throw exceptions.
//
// ----------------------------------------------------------------------------
template <typename PixelType, typename SizeType>
std::unique_ptr<PixelType[]> createBufferImage(
    SizeType width, SizeType height) noexcept {
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
        buffer[j*width + i] = std::numeric_limits<PixelType>::min();
      } else {
        buffer[j*width + i] = std::numeric_limits<PixelType>::max();
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
    ushort width;
    ushort height;
    std::vector<std::string> unsupported;
  };

  // Define the default values for the command line options
  CLIOptions user_options{
      false, // show_help
      false, // print_usage
      false, // show_version
      400,   // width
      400,   // height
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
      (
        clipp::option("-w", "--width")
          & clipp::opt_value("width", user_options.width)
          .doc("set the width of the image (default: 400, must be >= 10)"
          ),
        clipp::option("-h", "--height")
          & clipp::opt_value("height", user_options.height)
          .doc("set the height of the image (default: 400, must be >= 10)"
          ),
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

    // Check if the width and height values are valid
    if (user_options.width < 10 || user_options.height < 10) {
      std::cerr << exec_name
        << ": Error: 'width' and 'height' must be >= 10.\n";

      printShortHelp(exec_name.c_str());

      throw EXIT_FAILURE;
    }

    // Main code goes here ----------------------------------------------------

    // Define the image dimensions, pixel type and image type.
    // We will create a 2D monochrome image with 16-bit unsigned
    // integer pixel type.
    constexpr unsigned short int Dimensions = 2;
    using PixelType = uint16_t;
    using ImageType = itk::Image<PixelType, Dimensions>;

    // Define the filter type that will import the buffer image
    using ImportFilterType = itk::ImportImageFilter<PixelType, Dimensions>;

    auto importFilter = ImportFilterType::New();  // This is our filter object

    // Spcify the size of the image to be created
    ImportFilterType::SizeType size;
    size[0] = user_options.width;
    size[1] = user_options.height;

    // Specify the beginning of the image
    ImportFilterType::IndexType start;
    start.Fill(0);

    // Specify the image region
    ImportFilterType::RegionType region;
    region.SetIndex(start);
    region.SetSize(size);
    importFilter->SetRegion(region);

    // Specify the origin of the image
    const itk::SpacePrecisionType origin[Dimensions] = {0.0, 0.0};
    importFilter->SetOrigin(origin);

    // Specify the spacing of the image. We ste the spacing so the resulting
    // image will be 400 dpi.
    constexpr float x_spacing = 25.4 / 400.0;
    constexpr float y_spacing = 25.4 / 400.0;
    const itk::SpacePrecisionType spacing[Dimensions] = {x_spacing, y_spacing};
    importFilter->SetSpacing(spacing);

    // Create the buffer image
    auto buffer = createBufferImage<PixelType, size_t>(
      user_options.width, user_options.height
      );

    // Image filter will not take ownership of the buffer
    const bool own_data = false;

    // Set the buffer image to the filter
    importFilter->SetImportPointer(
      buffer.get(), user_options.width * user_options.height, own_data
      );

    // Connect the writer to the pipline
    using WriterType = itk::ImageFileWriter<ImageType>;
    using TIFFIOType = itk::TIFFImageIO;

    // Create IO object for TIFF file format
    auto tiffIO = TIFFIOType::New();
    tiffIO->SetPixelType(itk::IOPixelEnum::SCALAR);

    // Create the writer object
    auto writer = WriterType::New();
    writer->SetFileName("output.tif");
    writer->SetInput(importFilter->GetOutput());
    writer->SetImageIO(tiffIO);  // Explicitly set the IO object

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

inline void printShortHelp(const char *exec_name) noexcept {
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
) noexcept{
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
