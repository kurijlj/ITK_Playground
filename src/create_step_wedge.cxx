// ============================================================================
// create_step_wedge.cxx (ITK_Playground) - Create computational step wedge
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
// 2024-07-16 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
//
// * create_step_wedge.cxx: created.
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
#include <cstdlib>               // required by EXIT_SUCCESS, EXIT_FAILURE
#include <cmath>                 // required by log10f

// Standard Library headers
#include <array>
#include <exception>             // required by std::current_exception
#include <filesystem>            // required by std::filesystem
#include <iostream>              // required by cin, cout, cerr, ...
#include <string>                // required by std::string

// External libraries headers
#include <clipp.hpp>             // command line arguments parsing
#include <itkImage.h>            // required by itk::Image
#include <itkImageFileWriter.h>  // required for writing image data to file
#include <itkRGBPixel.h>         // required for handling RGB images
#include <itkSmartPointer.h>     // required by itk::SmartPointer
#include <itkTIFFImageIO.h>      // required for reading and writing TIFF images
#include <itkVector.h>           // required by itk::Vector (RGB pixel type)


// ============================================================================
// Namespace alias section
// ============================================================================


// ============================================================================
// User defined types section
// ============================================================================

using ComponentType = uint16_t;                   // 16-bit unsigned integer
                                                  // type
using RGB16Pixel = itk::RGBPixel<ComponentType>;  // RGB pixel with 16-bit
                                                  // unsigned integer values
using RGB16Image = itk::Image<RGB16Pixel, 2>;     // 2D RGB image with 16-bit
                                                  // unsigned integer pixel
                                                  // values


// ============================================================================
// Global constants section
// ============================================================================

static const std::string kAppName = "create_step_wedge";
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
// Function prototypes
// ============================================================================

// ----------------------------------------------------------------------------
// 'create_step_wedge' function
// ----------------------------------------------------------------------------
//
// Description:
// Create a computational step wedge image.
//
// Parameters:
//   od: Reference to the initializer list of the optical density values.
//   dpi: Image resolution in dots per inch.
//
// Returns:
//   A pointer to the created image.
//
// ----------------------------------------------------------------------------
RGB16Image::Pointer create_step_wedge(
  std::array<double, 21> od,
  uint16_t dpi = 400
  );


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
    auto image = create_step_wedge(
      std::array<double, 21> {0.04, 0.20, 0.35, 0.51, 0.65, 0.80, 0.94, 1.11,
        1.27, 1.43, 1.59, 1.73, 1.88, 2.02, 2.18, 2.32, 2.49, 2.64,
        2.79, 2.91, 3.08}
    );

    using WriterType = itk::ImageFileWriter<RGB16Image>;
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
// Utility function definitions
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


// ============================================================================
// Function definitions
// ============================================================================

RGB16Image::Pointer create_step_wedge(
    std::array<double, 21> od,
    uint16_t dpi
    ) {
  auto image = RGB16Image::New();

  // Step wedge dimensions in inches
  double stepWedgeWidth = 0.50;
  double stepWedgeHeight = 5.00;
  double imageWidth = stepWedgeWidth + 0.80 * stepWedgeWidth;
  double imageHeight = stepWedgeHeight + 0.80 * stepWedgeWidth;
  double firstStepWidth = 0.59;
  double stepWidth = 0.2;

  RGB16Image::RegionType region;
  RGB16Image::IndexType start;
  start[0] = 0;
  start[1] = 0;
  RGB16Image::SpacingType spacing;
  spacing[0] = 25.4 / static_cast<double>(dpi);
  spacing[1] = 25.4 / static_cast<double>(dpi);

  RGB16Image::SizeType size;
  uint16_t NumRows = static_cast<uint16_t> (round (imageHeight * dpi));
  uint16_t NumCols = static_cast<uint16_t> (round (imageWidth * dpi));
  size[0] = NumCols;
  size[1] = NumRows;

  region.SetSize(size);
  region.SetIndex(start);

  image->SetRegions(region);
  image->SetSpacing(spacing);
  image->Allocate();
  RGB16Image::PixelType pixelValue;
  pixelValue[0] = 65535;
  pixelValue[1] = 65535;
  pixelValue[2] = 65535;
  image->FillBuffer(pixelValue);

  // Step wedge origin and size in pixels
  uint16_t stepWedgeOrigin[2] = {
    static_cast<uint16_t> (round (0.40 * stepWedgeWidth * dpi)),
    static_cast<uint16_t> (round (0.40 * stepWedgeWidth * dpi))
    };
  uint16_t stepWedgeSize[2] = {
    static_cast<uint16_t> (round (stepWedgeWidth * dpi)),
    static_cast<uint16_t> (round (stepWedgeHeight * dpi))
    };

  // Individual step origin in pixels is the same as the step wedge origin
  uint16_t stepOrigin[2] = {
    stepWedgeOrigin[0],
    stepWedgeOrigin[1]
    };

  // Make a step wedge
  for (std::size_t step = 20; step > 0; --step) {
    
    // Calculate pixel values for each step
    pixelValue[0] = static_cast<uint16_t> (
      round (2140.00 + exp(-(od[step] - 6.966)/0.63))
      );
    pixelValue[1] = static_cast<uint16_t> (
      round (2140.00 + exp(-(od[step] - 6.966)/0.63))
      );
    pixelValue[2] = static_cast<uint16_t> (
      round (2140.00 + exp(-(od[step] - 6.966)/0.63))
      );
    
    // Calculate step size in pixels
    uint16_t stepSize[2] = {  // This is size of the last step
      stepWedgeSize[0],
      stepWedgeSize[1]
      };
    
    // Width of the in between steps is 0.2 inches
    if (20 > step) {
      stepSize[1] = static_cast<uint16_t> (round ((
        firstStepWidth
        + static_cast<double> (step) * stepWidth
        ) * dpi));
    }

    // Set pixel values for the step
    for (uint16_t x = stepOrigin[0]; x < stepOrigin[0] + stepSize[0];
         ++x) {
      for (uint16_t y = stepOrigin[1]; y < stepOrigin[1] + stepSize[1];
           ++y) {
        RGB16Image::IndexType pixelIndex;
        pixelIndex[0] = x;
        pixelIndex[1] = y;

        image->SetPixel(pixelIndex, pixelValue);
      }
    }
  }

  // We do first step separately because it has a different width theb the
  // rest of the steps

  // Calculate pixel values for the first step
  pixelValue[0] = static_cast<uint16_t> (
    round (2140.00 + exp(-(od[0] - 6.966)/0.63))
    );
  pixelValue[1] = static_cast<uint16_t> (
    round (2140.00 + exp(-(od[0] - 6.966)/0.63))
    );
  pixelValue[2] = static_cast<uint16_t> (
    round (2140.00 + exp(-(od[0] - 6.966)/0.63))
    );
    
  // Calculate size of the first step
  uint16_t stepSize[2] = {
    stepWedgeSize[0],
    static_cast<uint16_t> (round (firstStepWidth * dpi))
    };
  
  // Set pixel values for the first step
  for (uint16_t x = stepOrigin[0]; x < stepOrigin[0] + stepSize[0];
       ++x) {
    for (uint16_t y = stepOrigin[1]; y < stepOrigin[1] + stepSize[1];
         ++y) {
      RGB16Image::IndexType pixelIndex;
      pixelIndex[0] = x;
      pixelIndex[1] = y;

      image->SetPixel(pixelIndex, pixelValue);
    }
  }
    
  // Return the created image
  return image;
}
 