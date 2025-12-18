#include <cstdlib>

#include <itkBinaryThresholdImageFilter.h>
#include <itkFlipImageFilter.h>
#include <itkGDCMImageIO.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageMomentsCalculator.h>

int
main(int argc, char * argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " DicomFile " << std::endl;
		return EXIT_FAILURE;
	}

	//  Next, we instantiate the type to be used for storing the image once it is
	//  read into memory.
	using PixelType = signed short;
	constexpr unsigned int Dimension = 2;

	using ImageType = itk::Image<PixelType, Dimension>;

	// We use the image type for instantiating the series reader type and then we
	// construct one object of this class.
	using ReaderType = itk::ImageFileReader<ImageType>;

	auto reader = ReaderType::New();

	// A GDCMImageIO object is created and assigned to the reader.
	using ImageIOType = itk::GDCMImageIO;

	auto dicomIO = ImageIOType::New();

	reader->SetImageIO(dicomIO);

	reader->SetFileName(argv[1]);

	// We trigger the reader by invoking the Update() method. This
	// invocation should normally be done inside a try/catch block given
	// that it may eventually throw exceptions.
	try
	{
		reader->Update();
	}
	catch (const itk::ExceptionObject & ex)
	{
		std::cout << ex << std::endl;
		return EXIT_FAILURE;
	}

	// ITK internally queries GDCM and obtains all the DICOM tags from the file
	// headers. The tag values are stored in the MetaDataDictionary
	// which is a general-purpose container for \{key,value\} pairs. The Metadata
	// dictionary can be recovered from any ImageIO class by invoking the
	// GetMetaDataDictionary() method.
	using DictionaryType = itk::MetaDataDictionary;

	const DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();

	// In this example, we are only interested in the DICOM tags that can be
	// represented as strings. Therefore, we declare a MetaDataObject of
	// string type in order to receive those particular values.
	using MetaDataStringType = itk::MetaDataObject<std::string>;

	// The metadata dictionary is organized as a container with its corresponding
	// iterators. We can therefore visit all its entries by first getting access to
	// its Begin() and End() methods.
	auto itr = dictionary.Begin();
	auto end = dictionary.End();

	// We are now ready for walking through the list of DICOM tags. For this
	// purpose we use the iterators that we just declared. At every entry we
	// attempt to convert it into a string entry by using the dynamic_cast
	// based on RTTI information. The
	// dictionary is organized like a std::map structure, so we should use
	// the first and second members of every entry in order
	// to get access to the \{key,value\} pairs.
	while (itr != end)
	{
		itk::MetaDataObjectBase::Pointer entry = itr->second;

		MetaDataStringType::Pointer entryvalue = dynamic_cast<MetaDataStringType *>(entry.GetPointer());

		if (entryvalue)
		{
			std::string tagkey = itr->first;
			std::string tagvalue = entryvalue->GetMetaDataObjectValue();
			std::cout << tagkey << " = " << tagvalue << std::endl;
		}

		++itr;
	}

	//  It is also possible to query for specific entries instead of reading all of
	//  them as we did above. In this case, the user must provide the tag
	//  identifier using the standard DICOM encoding. The identifier is stored in a
	//  string and used as key in the dictionary.
	std::string entryId = "0010|0010";

	auto tagItr = dictionary.Find(entryId);

	if (tagItr == end)
	{
		std::cerr << "Tag " << entryId;
		std::cerr << " not found in the DICOM header" << std::endl;
		return EXIT_FAILURE;
	}

	// Since the entry may or may not be of string type we must again use a
	// dynamic_cast in order to attempt to convert it to a string dictionary
	// entry. If the conversion is successful, we can then print out its content.
	MetaDataStringType::ConstPointer entryvalue = dynamic_cast<const MetaDataStringType *>(tagItr->second.GetPointer());

	if (entryvalue)
	{
		std::string tagvalue = entryvalue->GetMetaDataObjectValue();
		std::cout << "Patient's Name (" << entryId << ") ";
		std::cout << " is: " << tagvalue << std::endl;
	}
	else
	{
		std::cerr << "Entry was not of string type" << std::endl;
		return EXIT_FAILURE;
	}

	// This type of functionality will probably be more useful when provided
	// through a graphical user interface. For a full description of the DICOM
	// dictionary please look at the following file.

	using SpacingType = ImageType::SpacingType;
	const SpacingType & pixelSpacing = reader->GetOutput()->GetSpacing();
	std::cout << "Pixel spacing: " << pixelSpacing << "\n";

	using FilterType = itk::BinaryThresholdImageFilter<ImageType, ImageType>;
	auto filter = FilterType::New();
	filter->SetInput(reader->GetOutput());
	filter->SetUpperThreshold(32767);
	filter->SetLowerThreshold(65535);
	filter->SetOutsideValue(0);
	filter->SetInsideValue(65535);
	filter->Update();

	// Software Guide : BeginCodeSnippet
  	using ImageCalculatorType = itk::ImageMomentsCalculator<ImageType>;
 
	auto calculator1 = ImageCalculatorType::New();
	calculator1->SetImage(filter->GetOutput());
	try
	{
		calculator1->Compute();
	}
	catch(const itk::ExceptionObject & e)
	{
		std::cerr << "Can not calculate moments for original ...\n";
		std::cerr << "Error: " << e << std::endl;
		return EXIT_FAILURE;
	}
	calculator1->Print(std::cout, 0);

	ImageCalculatorType::VectorType center1 = calculator1->GetCenterOfGravity();
	std::cout << "Center1: " << center1[0] << ", " << center1[1] << "\n";

	try
	{
		itk::WriteImage(filter->GetOutput(), "output.tif");
	}
	catch (const itk::ExceptionObject & e)
	{
		std::cerr << "Error: " << e << std::endl;
		return EXIT_FAILURE;
	}

	using FlipImageFilterType = itk::FlipImageFilter<ImageType>;

	auto flipFilter = FlipImageFilterType::New();
	flipFilter->SetInput(filter->GetOutput());

	FlipImageFilterType::FlipAxesArrayType flipAxes;
	flipAxes[0] = true;
	flipAxes[1] = true;

	flipFilter->SetFlipAxes(flipAxes);
	flipFilter->Update();

	auto calculator2 = ImageCalculatorType::New();
	calculator2->SetImage(flipFilter->GetOutput());
	try
	{
		calculator2->Compute();
	}
	catch(const itk::ExceptionObject & e)
	{
		std::cerr << "Can not calculate moments for flip ...\n";
		std::cerr << "Error: " << e << std::endl;
		return EXIT_FAILURE;
	}
	calculator2->Print(std::cout, 0);

	ImageCalculatorType::VectorType center2 = calculator2->GetCenterOfGravity();
	std::cout << "Center2: " << center2[0] << ", " << center2[1] << "\n";

	auto flipped = ImageType::New();

	flipped->SetRegions(flipFilter->GetOutput()->GetLargestPossibleRegion());
  	flipped->Allocate();
	flipped->SetSpacing(pixelSpacing);

	itk::ImageRegionConstIterator<ImageType> inputIterator(
		flipFilter->GetOutput(),
		flipFilter->GetOutput()->GetLargestPossibleRegion()
	);
	itk::ImageRegionIterator<ImageType> outputIterator(
		flipped,
		flipped->GetLargestPossibleRegion()
	);

	while (!inputIterator.IsAtEnd())
	{
		outputIterator.Set(inputIterator.Get());
		++inputIterator;
		++outputIterator;
	}

	calculator2->SetImage(flipped);
	calculator2->Compute();
	calculator2->Print(std::cout, 0);

	try
	{
		itk::WriteImage(flipFilter->GetOutput(), "output_flipped.tif");
	}
	catch (const itk::ExceptionObject & error)
	{
		std::cerr << "Error: " << error << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// End of `shifts_calculator.cxx'