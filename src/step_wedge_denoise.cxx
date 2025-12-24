#include <cstdlib>

#include <filesystem>
#include <iostream>
#include <string>
#include <variant>

#include <itkBilateralImageFilter.h>
#include <itkImage.h>
#include <itkImageAdaptor.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkTIFFImageIO.h>

enum class ColorChannel { Red, Green, Blue };

using RGB16Pixel = itk::RGBPixel<uint16_t>;

template <ColorChannel channel>
class RGB16ColorChannelAccessor {
public:
  using InternalType = RGB16Pixel;
  using ExternalType = uint16_t;

  static ExternalType
  Get(const InternalType &input) {
    switch (channel) {
      case ColorChannel::Red:
        return input.GetRed();
      case ColorChannel::Green:
        return input.GetGreen();
      case ColorChannel::Blue:
        return input.GetBlue();
      default:
        return 0;  // Should never reach this point
    }
  }
};

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem; // Filesystem alias

	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " [TIFF_FILE] " << std::endl;
		return EXIT_FAILURE;
	}
    
    const std::string input_file{argv[1]};

    // Decompose the input file name into the base name and the extension
    std::string out_base_name
      = fs::path(input_file).stem().string();
    std::string out_extension
      = fs::path(input_file).extension().string();

    // Instantiate the TIFF image reader
    auto tiffImageIO = itk::TIFFImageIO::New();

    // Check if we are dealing with a regular TIFF image
    if (!tiffImageIO->CanReadFile(input_file.c_str())) {
        std::cerr << ": File is not a regular TIFF image: "
            << input_file
            << "\n";
        return EXIT_FAILURE;
    }

    // Set the file name and read the image information
    tiffImageIO->SetFileName(input_file);
    tiffImageIO->ReadImageInformation();

    // Check if we are dealing with an compressed image
    if (tiffImageIO->ReadCompressionFromImage() != 1) {  // 1 = no compression
        std::cerr << ": File is compressed: "
            << input_file
            << "\n";
        return EXIT_FAILURE;
    }

    // Check if we are dealing with the RGB image
    if (tiffImageIO->ReadSamplesPerPixelFromImage() != 3) {
        std::cerr << ": File is not an RGB image: "
            << input_file
            << "\n";
        return EXIT_FAILURE;
    }

    // Check if we are dealing with 16-bit image
    if (tiffImageIO->ReadBitsPerSampleFromImage() != 16) {
      std::cerr << ": File is not a 16-bit image: "
        << input_file
        << "\n";
      return EXIT_FAILURE;
    }

    // Define accessor and utility classes for accessing the color channels
    // Define image types for the input and output images
	constexpr unsigned int Dimension = 2;
    using RGB16Image = itk::Image<RGB16Pixel, Dimension>;
    using Mono16Image = itk::Image<uint16_t, Dimension>;

    // Define the image reader and writer types
    using RGB16Reader = itk::ImageFileReader<RGB16Image>;
    using Mono16Writer = itk::ImageFileWriter<Mono16Image>;

    // Define the adaptors for the color channels
    using RedChannelAdaptor = itk::ImageAdaptor<
        RGB16Image,
        RGB16ColorChannelAccessor<ColorChannel::Red>
    >;
    using RedChannelRescalerType
      = itk::RescaleIntensityImageFilter<RedChannelAdaptor, Mono16Image>;

    // Define the bilateral filter type
    using DenoisingFilterType
        = itk::BilateralImageFilter<Mono16Image, Mono16Image>;

    // Instantiate objects and connect them
    auto reader = RGB16Reader::New();
    reader->SetFileName(input_file);

    auto red_channel = RedChannelAdaptor::New();
    red_channel->SetImage(reader->GetOutput());

    auto red_rescaler = RedChannelRescalerType::New();
    red_rescaler->SetOutputMinimum(std::numeric_limits<uint16_t>::min());
    red_rescaler->SetOutputMaximum(std::numeric_limits<uint16_t>::max());
    red_rescaler->SetInput(red_channel);

    double rangeSigma{9.0};
    double domainSigmas[Dimension] = {10.0, 10.0};
    auto red_filter = DenoisingFilterType::New();
    red_filter->SetInput(red_rescaler->GetOutput());
    red_filter->SetDomainSigma(domainSigmas);
    red_filter->SetRangeSigma(rangeSigma);

    auto red_writer = Mono16Writer::New();
    red_writer->SetFileName((out_base_name + "_R" + out_extension).c_str());
    red_writer->SetInput(red_filter->GetOutput());


    // Write channels to files
    try {
        red_writer->Update();
    } catch (const itk::ExceptionObject & error) {
        std::cerr << ": Error writing file: '"
            << out_base_name << "_R" << out_extension
            << "'. "
            << error
            << "\n";
        throw EXIT_FAILURE;
    }

    // Return success
    return EXIT_SUCCESS;
}

// End of file `src/step_wedge_denoise.cxx'
