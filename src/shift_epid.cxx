// ============================================================================
// shift_epid.cxx (ITK_Playground) - Translate EPID image for a given amount
//
//  Copyright (C) 2026 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
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
// 2026-01-19 Ljubomir Kurij <ljubomir_kurij@protonmail.com>
//
// * shift_epid.cxx: created.
//
// ============================================================================


// ============================================================================
// Preprocessor directives section
// ============================================================================


// ============================================================================
// Headers include section
// ============================================================================

// Related Header -------------------------------------------------------------

// "C" Standard Library Headers -----------------------------------------------
#include <cmath>
#include <cstdlib>

// "C++" Standard Library Headers ---------------------------------------------
#include <filesystem>
#include <iostream>
#include <string>

// External Library Headers ----------------------------------------------------

// clipp
#include "clipp.hpp"

// ITK
#include <itkGDCMImageIO.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>
#include <itkTranslationTransform.h>


// ============================================================================
// Global constants section
// ============================================================================

static constexpr int DEFAULT_PAGE_IDENT{0};
static constexpr int DEFAULT_PAGE_WIDTH{79};

static constexpr auto APP_NAME{"shift_epid"};
static constexpr auto VERSION_STRING{"0.1.0"};
static constexpr auto YEAR_STRING{"2026"};
static constexpr auto AUTHOR_NAME{"Ljubomir Kurij"};
static constexpr auto AUTHOR_EMAIL{"ljubomir_kurij@protonmail.com"};
static constexpr auto APP_DOC{"\
Perform EPID acquired image shifts for a given directional vector.\n\n\
Mandatory arguments to long options are mandatory for short options too.\n"};
static constexpr auto HELP_OPTION_DOC = "\
show this help message and exit";
static constexpr auto USAGE_OPTION_DOC = "\
give a short usage message";
static constexpr auto VERISON_OPTION_DOC = "print program version";
static constexpr auto INPUT_FILE_DOC = "\
EPID image DICOM file";
static constexpr auto XSHIFT_OPTION_DOC{"\
set shift alongside X axis (default: 0.0, must be > -200.0 mm and < +200.0 mm)"
};
static constexpr auto YSHIFT_OPTION_DOC{"\
set shift alongside Y axis (default: 0.0, must be > -200.0 mm and < +200.0 mm)"
};
static constexpr auto OUTPUT_FILE_OPTION_DOC{"\
where to save shifted EPID image (default: INPUT_FILE_shifted.[DCM|dcm])"};
static constexpr auto LICENSE_STRING{"\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n"};


// ============================================================================
// CLI Utilities Froward Declaration Section
// ============================================================================

struct CLIOptions {
	std::vector<std::string> m_Unsupported;
	std::string              m_InputFile;
	std::string              m_OutputFile;
	float                    m_XShift;
	float                    m_YShift;
	bool                     m_ShowHelp;
	bool                     m_PrintUsage;
	bool                     m_ShowVersion;
};

void printShortHelp(const std::string_view& appName);

void handleUnsupportedOptions(
    const std::string_view&    appName,
    const CLIOptions& userSupplied
);

void printUsage(
    const std::string_view&      appName,
    const clipp::group&          appOptions,
    const clipp::doc_formatting& formatting
);

void printVersionInfo(
    const std::string_view& appName,
    const std::string_view& versionString,
    const std::string_view& yearString,
    const std::string_view& copyrightHolder,
    const std::string_view& license
);

void showHelp(
    const std::string_view&      appName,
    const clipp::group&          appOptions,
    const std::string_view&      appDoc,
    const std::string_view&      authorEmail,
    const clipp::doc_formatting& formatting
);

// ============================================================================
// Main Function Section
// ============================================================================

int main(int argc, char* argv[])
{
	// Aliases
    namespace fs = std::filesystem;

	constexpr float kShiftLimit{200.0};

    // Define the default values for the command line options
    static CLIOptions userOptionValues
    {
        std::vector<std::string> {},  // m_Unsupported
        std::string{},                // m_InputFile
        std::string{},   // m_OutputFile
        0.0,                          // m_XShift
        0.0,                          // m_YShift
        false,                        // m_ShowHelp
        false,                        // m_PrintUsage
        false                         // m_ShowVersion
    };
	
	// Option filters definitions
	auto istarget = clipp::match::prefix_not("-");  // Filter out strings that
													 // start with '-' (options)

    // Define CLI options we want to  support
    auto appOptions = (
        (
            (
                clipp::option("-h", "--help")
                    .set(userOptionValues.m_ShowHelp)
            ).doc(HELP_OPTION_DOC),
            (
                clipp::option("--usage")
                    .set(userOptionValues.m_PrintUsage)
            ).doc(USAGE_OPTION_DOC),
            (
                clipp::option("-V", "--version")
                    .set(userOptionValues.m_ShowVersion)
            ).doc(VERISON_OPTION_DOC)
        ).doc("general options:"),
		(
        	clipp::opt_value(
				istarget,
				"INPUT_FILE",
				userOptionValues.m_InputFile
			).doc(INPUT_FILE_DOC),
        	clipp::option("-x", "--x-shift")
				& clipp::opt_value("X shift", userOptionValues.m_XShift)
					.doc(XSHIFT_OPTION_DOC),
			clipp::option("-y", "--y-shift")
				& clipp::opt_value("Y shift", userOptionValues.m_YShift)
					.doc(YSHIFT_OPTION_DOC),
			clipp::option("-o", "--output-file")
				& clipp::opt_value("OUTPUT_FILE", userOptionValues.m_OutputFile)
					.doc(OUTPUT_FILE_OPTION_DOC)
		).doc("EPID options"),
        clipp::any_other(userOptionValues.m_Unsupported)
    );

    // Determine the exec name under wich program is beeing executed
    std::string execName = fs::path(argv[0]).filename().string();

    // Set default CLI help formatting
    auto appFormatting = clipp::doc_formatting {}
        .first_column(DEFAULT_PAGE_IDENT)
        .last_column(DEFAULT_PAGE_WIDTH);

    // Parse command line options
    auto result = clipp::parse(argc, argv, appOptions);

    // Check for the unsupported options --------------------------------------
    if (!userOptionValues.m_Unsupported.empty())
    {
        handleUnsupportedOptions(
            execName,
            userOptionValues
        );
        return EXIT_FAILURE;
    }

    // Check for high priority switches ---------------------------------------
    // (i.e. '--help', '--usage', '--version')
    else if (userOptionValues.m_ShowHelp)
    {
        showHelp(
            execName,
            appOptions,
            APP_DOC,
            AUTHOR_EMAIL,
            appFormatting
        );
        return EXIT_SUCCESS;
    }
    else if (userOptionValues.m_PrintUsage)
    {
        // Check if the usage switch was triggered. Usge switch has the second
        // highest priority, so if it is triggered we don't need to check
        // anything else.
        printUsage(APP_NAME, appOptions, appFormatting);
        return EXIT_SUCCESS;
    }
    else if (userOptionValues.m_ShowVersion)
    {
        // Check if the version switch was triggered. Version switch has the
        // third highest priority.
        printVersionInfo(
            APP_NAME,
            VERSION_STRING,
            YEAR_STRING,
            AUTHOR_EMAIL,
            LICENSE_STRING
        );
        return EXIT_SUCCESS;
    }

    // No high priority switch was passed -------------------------------------

    // Check if the input file was passed
    if (userOptionValues.m_InputFile.empty()) {
    	std::cout << "Usage: ";
    	printUsage(APP_NAME, appOptions, appFormatting);

    	std::cout << "\n";

    	// Print short help message
    	printShortHelp(execName);

    	return EXIT_FAILURE;
    }

    // Check if the file exists
    if (!fs::exists (userOptionValues.m_InputFile)) {
    	std::cerr << APP_NAME
    		<< ": File does not exist: "
    		<< userOptionValues.m_InputFile
    		<< "\n";
    	return EXIT_FAILURE;
    }

    // Check if the file is a regular file
    if (!fs::is_regular_file (userOptionValues.m_InputFile)) {
    	std::cerr << APP_NAME
    		<< ": Not a regular file: "
    		<< userOptionValues.m_InputFile
    		<< std::endl;
    	return EXIT_FAILURE;
    }

    // Check if the file is empty
    if (0 == fs::file_size (userOptionValues.m_InputFile)) {
    	std::cerr << APP_NAME
    		<< ": Empty file: "
    		<< userOptionValues.m_InputFile
    		<< std::endl;
    	return EXIT_FAILURE;
    }

    // Set up DICOM IO
    auto dicomIO = itk::GDCMImageIO::New();

	// Check if the file is a valid DICOM file
	if (!dicomIO->CanReadFile(userOptionValues.m_InputFile.c_str()))
	{
	    std::cerr << APP_NAME
	        << ": Not a valid DICOM file: "
	        << userOptionValues.m_InputFile
	        << "\n";
	    return EXIT_FAILURE;
	}

	// Check if user supplied a valid value for shift along X axis
	if (kShiftLimit < std::abs(userOptionValues.m_XShift))
	{
	    std::cerr << APP_NAME
	        << ": X shift out of limits. "
			<< "Expected -200.0 mm < shift < +200.0 mm. Got: "
	        << userOptionValues.m_XShift
	        << "\n";
	    return EXIT_FAILURE;
	}

	// Check if user supplied a valid value for shift along Y axis
	if (kShiftLimit < std::abs(userOptionValues.m_YShift))
	{
	    std::cerr << APP_NAME
	        << ": Y shift out of limits. "
			<< "Expected -200.0 mm < shift < +200.0 mm. Got: "
	        << userOptionValues.m_YShift
	        << "\n";
	    return EXIT_FAILURE;
	}

    // Determine the output file name if not user supplied
    if (userOptionValues.m_OutputFile.empty()) {
    	// Decompose the input file name into the base name and the extension
    	std::string outBaseName
    		= fs::path(userOptionValues.m_InputFile).stem().string();
    	std::string outExtension
    		= fs::path(userOptionValues.m_InputFile).extension().string();

		// We construct output file name by appending _shifted to the input
		// file's base name
		userOptionValues.m_OutputFile = outBaseName + "_shifted" + outExtension;
    }

	// Proceed with normal execution ------------------------------------------
	static constexpr unsigned int Dimension = 2;
	using PixelType = short;  // Typical for CT/MR DICOM
	using ImageType = itk::Image<PixelType, Dimension>;

    // Read the input DICOM image
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(userOptionValues.m_InputFile);
    reader->SetImageIO(dicomIO);

    try
    {
        reader->Update();
    }
    catch (const itk::ExceptionObject& ex)
    {
        std::cerr << "Error reading input: " << ex << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer inputImage = reader->GetOutput();

    ImageType::RegionType region = inputImage->GetLargestPossibleRegion();
    ImageType::SizeType size = region.GetSize();

    std::cout << "Input image info:\n"
              << "  Size: " << size << "\n"
              << "  Spacing: " << inputImage->GetSpacing() << " mm\n"
              << "  Origin: " << inputImage->GetOrigin() << " mm\n";

    // Estimate background by sampling corner pixels
    ImageType::IndexType idx;
    long sumCorners = 0;

    // Top-mid pixel
    idx[0] = size[0]/2 - 1;
    idx[1] = 0;
    sumCorners += inputImage->GetPixel(idx);

    // Right-mid pixel
    idx[0] = size[0] - 1;
    idx[1] = size[1]/2 - 1;
    sumCorners += inputImage->GetPixel(idx);

    // Bottom-mid pixel
    idx[0] = size[0]/2 - 1;
    idx[1] = size[1] - 1;
    sumCorners += inputImage->GetPixel(idx);

    // Left-mid pixel
    idx[0] = 0;
    idx[1] = size[1]/2 - 1;
    sumCorners += inputImage->GetPixel(idx);

    PixelType backgroundValue = static_cast<PixelType>(sumCorners / 4);
    std::cout << "  Estimated background: " << backgroundValue << "\n";

    // Create translation transform
    using TransformType = itk::TranslationTransform<double, Dimension>;
    auto transform = TransformType::New();

    TransformType::OutputVectorType offset;
    offset[0] = userOptionValues.m_XShift;
    offset[1] = userOptionValues.m_YShift;
    transform->SetOffset(offset);

    std::cout << "Applying translation: ["
              << offset[0] << ", " << offset[1] << "] mm\n";

    // Create nearest neighbor interpolator to preserve original pixel values
    using InterpolatorType
        = itk::NearestNeighborInterpolateImageFunction<ImageType, double>;
    auto interpolator = InterpolatorType::New();

    // Set up resampling filter
    using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
    auto resampler = ResampleFilterType::New();

    resampler->SetInput(inputImage);
    resampler->SetTransform(transform);
    resampler->SetInterpolator(interpolator);

    // Preserve original image geometry
    resampler->SetSize(inputImage->GetLargestPossibleRegion().GetSize());
    resampler->SetOutputSpacing(inputImage->GetSpacing());
    resampler->SetOutputOrigin(inputImage->GetOrigin());
    resampler->SetOutputDirection(inputImage->GetDirection());

    // Use estimated background for pixels outside the original image
    resampler->SetDefaultPixelValue(backgroundValue);

    try
    {
        resampler->Update();
    }
    catch (const itk::ExceptionObject& ex)
    {
        std::cerr << "Error during resampling: " << ex << std::endl;
        return EXIT_FAILURE;
    }

    // Write output DICOM
    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetFileName(userOptionValues.m_OutputFile);
    writer->SetImageIO(dicomIO);
    writer->SetInput(resampler->GetOutput());
    writer->UseInputMetaDataDictionaryOff();  // Use metadata from dicomIO

    try
    {
        writer->Update();
        std::cout << "Successfully wrote: "
			<< userOptionValues.m_OutputFile << "\n";
    }
    catch (const itk::ExceptionObject& ex)
    {
        std::cerr << "Error writing output: " << ex << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


// ============================================================================
// CLI Utilities Implementation Section
// ============================================================================

void
printShortHelp(const std::string_view& appName)
{
    std::cout << "Try '" << appName << " --help' for more information.\n";
}

void
handleUnsupportedOptions(
    const std::string_view&    appName,
    const CLIOptions& userSupplied
) {
    std::cerr << appName << ": Unsupported options: ";
    for (const auto& val : userSupplied.m_Unsupported)
    {
        std::cerr << val << " ";
    }
    std::cerr << "\n";
}

void
printUsage(
    const std::string_view&      appName,
    const clipp::group&          appOptions,
    const clipp::doc_formatting& formatting
) {
    std::cout << clipp::usage_lines(
            appOptions,
            std::string{appName},
            formatting
        ) << "\n";
}

void
printVersionInfo(
    const std::string_view& appName,
    const std::string_view& versionString,
    const std::string_view& yearString,
    const std::string_view& copyrightHolder,
    const std::string_view& license
)
{
    std::cout << appName
        << " "
        << versionString
        << " Copyright (C) "
        << yearString
        << " "
        << copyrightHolder
        << "\n"
        << license;
}

void
showHelp(
    const std::string_view&      appName,
    const clipp::group&          appOptions,
    const std::string_view&      appDoc,
    const std::string_view&      authorEmail,
    const clipp::doc_formatting& formatting
)
{
    clipp::man_page man;

    std::string usageLines = clipp::usage_lines(
        appOptions,
        std::string{appName},
        formatting
    ).str();
    man.prepend_section("USAGE", usageLines);
    man.append_section("", std::string{appDoc});
    man.append_section(
        "",
        clipp::documentation(
            appOptions,
            formatting
        ).str()
    );
    man.append_section(
        "",
        "Report bugs to <"
            + std::string{authorEmail}
            + ">."
    );

    std::cout << man;
}


// End of `shift_epid.cxx'
