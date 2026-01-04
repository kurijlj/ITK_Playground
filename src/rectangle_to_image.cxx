#include <cstdlib>

#include <itkImageFileWriter.h>
#include <itkPolygonSpatialObject.h>
#include <itkSpatialObjectToImageFilter.h>
#include <itkTIFFImageIO.h>
 
int
main(int argc, char * argv[])
{
  using PixelType = unsigned short int;
  constexpr unsigned int Dimension = 2;
 
  using ImageType = itk::Image<PixelType, Dimension>;
  using PolygonType = itk::PolygonSpatialObject<Dimension>;
  using SpatialObjectToImageFilterType =
    itk::SpatialObjectToImageFilter<PolygonType, ImageType>;
 
  auto imageFilter = SpatialObjectToImageFilterType::New();

  ImageType::SizeType size;
  size[0] = 400;
  size[1] = 400;
 
  imageFilter->SetSize(size);
 
  ImageType::SpacingType spacing;
  spacing[0] = 25.4 / size[0];
  spacing[1] = 25.4 / size[1];
 
  imageFilter->SetSpacing(spacing);
  imageFilter->SetInsideValue(65535);
  imageFilter->SetOutsideValue(0);

  typename PolygonType::PointType point;
  typename PolygonType::PolygonPointType polygonPoint;
 
  auto polygon = PolygonType::New();
  
  // Point coordinates are in physical space (inches)
  point[0] = 6.35;
  point[1] = 6.35;
  polygonPoint.SetPositionInObjectSpace(point);
  polygon->GetPoints().push_back(polygonPoint);
  point[0] = 19.05;
  point[1] = 6.35;
  polygonPoint.SetPositionInObjectSpace(point);
  polygon->GetPoints().push_back(polygonPoint);
  point[0] = 19.05;
  point[1] = 19.05;
  polygonPoint.SetPositionInObjectSpace(point);
  polygon->GetPoints().push_back(polygonPoint);
  point[0] = 6.35;
  point[1] = 19.05;
  polygonPoint.SetPositionInObjectSpace(point);
  polygon->GetPoints().push_back(polygonPoint);

  polygon->SetIsClosed(true);
  polygon->Update();
 
  std::cout << "Polygon Perimeter = "
            << polygon->MeasurePerimeterInObjectSpace() << std::endl;
  std::cout << "Polygon Area      = " << polygon->MeasureAreaInObjectSpace()
            << std::endl;

  // Get bounding box in physical space
  auto boundingBox = polygon->GetMyBoundingBoxInObjectSpace();
  std::cout << "Bounding Box Min: [" << boundingBox->GetMinimum()[0] << ", "
            << boundingBox->GetMinimum()[1] << "]" << std::endl;
  std::cout << "Bounding Box Max: [" << boundingBox->GetMaximum()[0] << ", "
            << boundingBox->GetMaximum()[1] << "]" << std::endl;

  imageFilter->SetInput(polygon);
  imageFilter->Update();

  // Convert physical bounding box to image region
  auto image = imageFilter->GetOutput();

  ImageType::PointType minPoint, maxPoint;
  minPoint[0] = boundingBox->GetMinimum()[0];
  minPoint[1] = boundingBox->GetMinimum()[1];
  maxPoint[0] = boundingBox->GetMaximum()[0];
  maxPoint[1] = boundingBox->GetMaximum()[1];

  ImageType::IndexType minIndex, maxIndex;
  image->TransformPhysicalPointToIndex(minPoint, minIndex);
  image->TransformPhysicalPointToIndex(maxPoint, maxIndex);

  ImageType::RegionType boundingRegion;
  ImageType::IndexType regionIndex;
  ImageType::SizeType regionSize;

  regionIndex[0] = minIndex[0];
  regionIndex[1] = minIndex[1];
  regionSize[0] = maxIndex[0] - minIndex[0] + 1;
  regionSize[1] = maxIndex[1] - minIndex[1] + 1;

  boundingRegion.SetIndex(regionIndex);
  boundingRegion.SetSize(regionSize);

  std::cout << "Image Region: " << boundingRegion << std::endl;

  using WriterType = itk::ImageFileWriter<ImageType>;
  using TIFFIOType = itk::TIFFImageIO;

  auto tiffIO = TIFFIOType::New();
  tiffIO->SetPixelType(itk::IOPixelEnum::RGB);

  auto writer = WriterType::New();
  writer->SetFileName("polygon_image.tiff");
  writer->SetInput(imageFilter->GetOutput());
  writer->SetImageIO(tiffIO);
 
  try
  {
    writer->Update();
  }
  catch (const itk::ExceptionObject & excp)
  {
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
  }
  // Software Guide : EndCodeSnippet
 
 
  return EXIT_SUCCESS;
}