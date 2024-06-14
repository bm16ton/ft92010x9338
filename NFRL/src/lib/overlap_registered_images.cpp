/*******************************************************************************
License:
This software was developed at the National Institute of Standards and
Technology (NIST) by employees of the Federal Government in the course
of their official duties. Pursuant to title 17 Section 105 of the
United States Code, this software is not subject to copyright protection
and is in the public domain. NIST assumes no responsibility  whatsoever for
its use by other parties, and makes no guarantees, expressed or implied,
about its quality, reliability, or any other characteristic.

This software has been determined to be outside the scope of the EAR
(see Part 734.3 of the EAR for exact details) as it has been created solely
by employees of the U.S. Government; it is freely distributed with no
licensing requirements; and it is considered public domain. Therefore,
it is permissible to distribute this software as a free download from the
internet.

Disclaimer:
This software was developed to promote biometric standards and biometric
technology testing for the Federal Government in accordance with the USA
PATRIOT Act and the Enhanced Border Security and Visa Entry Reform Act.
Specific hardware and software products identified in this software were used
in order to perform the software development.  In no case does such
identification imply recommendation or endorsement by the National Institute
of Standards and Technology, nor does it imply that the products and equipment
identified are necessarily the best available for the purpose.
*******************************************************************************/
#include "opencv_procs.h"
#include "overlap_registered_images.h"


namespace NFRL {

#define ROI_THRESH 12   // minimum width and height thresholds

/** Initialization function that resets all values. */
void OverlapRegisteredImages::Init() {
  _dilationKernelParams.size = -1;
  _dilationKernelParams.type = -1;
}

/** @brief Supports copy-constructor. */
void OverlapRegisteredImages::Copy( const OverlapRegisteredImages& aCopy )
{
  _dilationKernelParams.size = aCopy._dilationKernelParams.size;
  _dilationKernelParams.type = aCopy._dilationKernelParams.type;
}

/** @brief Default constructor.  Calls Init(). */
OverlapRegisteredImages::OverlapRegisteredImages()
{
  Init();
}

/** @brief Copy constructor.
 *
 * This is called when passing the object by value as parameter to
 * OverlapRegisteredImages constructor.
 */
OverlapRegisteredImages::OverlapRegisteredImages(
    const OverlapRegisteredImages& aCopy )
{
  Copy( aCopy );
}

/**
 * @brief Images are required to be already registered.
 *
 * Moving image is translated and rotated to align with the fixed image.
 * 
 *   1. Binarize each padded image using OTSU method
 *   2. Sum the binary images to determine the overlap
 *   3. Calculate the ROI that is the area to crop for final, registered images
 * 
 * @param img1 - padded, must be same size as img2
 * @param img2 - padded, must be same size as img1
 */
OverlapRegisteredImages::OverlapRegisteredImages( cv::Mat img1, cv::Mat img2 )
{
  // Opencv support,  MORPH_ELLIPSE  MORPH_CROSS  MORPH_RECT
  _dilationKernelParams.type = cv::MORPH_RECT;
  _dilationKernelParams.size = 1;

  try {
    cv::Mat img1Binary, img2Binary;
    CVops::binarize_image_via_otsu_threshold( img1, img1Binary,
                                              _max_BINARY_value );
    CVops::binarize_image_via_otsu_threshold( img2, img2Binary,
                                              _max_BINARY_value );

    // "Sum" the two image binaries to calculate the overlap.
    cv::Mat sumOverlapOfRegisteredBinaries =
      cv::Mat::zeros(img1Binary.rows, img1Binary.cols, CV_8UC1);
    CVops::sum_two_binary_images( img1Binary, img2Binary,
                                  sumOverlapOfRegisteredBinaries );

    cv::Mat sumBinariesInverted;
    cv::bitwise_not( sumOverlapOfRegisteredBinaries, sumBinariesInverted );

    cv::Mat sumBinariesDilate;
    CVops::image_dilate( sumBinariesInverted, sumBinariesDilate,
                         _dilationKernelParams.size,
                         _dilationKernelParams.type );

    // Save the blob
    std::vector<int> param(1);
    param[0] = cv::IMWRITE_PNG_STRATEGY_DEFAULT;
    cv::imencode(".png", sumBinariesDilate, _vecPngBlob, param);


    cv::Mat nonZeroPoints;
    cv::findNonZero( sumBinariesDilate, nonZeroPoints );
    _minRect = cv::boundingRect( nonZeroPoints );
    if( isRegionOfInterestEmpty() )
    {
      throw NFRL::Miscue( "Registered images overlap region is empty." );
    }
    std::string errMsg;
    if( isRegionOfInterestBelowThresh( errMsg ) )
    {
      throw NFRL::Miscue( errMsg );
    }

  }
  catch( const cv::Exception& ex ) {
    std::string err{"OverlapRegisteredImages, cannot calc image-crop ROI: "};
    err.append( ex.what() );
    throw NFRL::Miscue( err );
  }
}

/**
 * @return rectangle of overlap for cropping of source images
 */
cv::Rect OverlapRegisteredImages::getRegionOfInterest() const
{
  return _minRect;
}

/**
 * @return rectangle top-left and bottom-right corners of overlap for cropping
 *         of source images
 */
std::vector<std::string> OverlapRegisteredImages::getRegionOfInterestCorners() const
{
  std::string s{""};
  std::vector<std::string> pts{};
  s = std::to_string( _minRect.tl().x ) + "," + std::to_string( _minRect.tl().y );
  pts.push_back(s);
  s = std::to_string( _minRect.br().x ) + "," + std::to_string( _minRect.br().y );
  pts.push_back(s);

  return pts;
}

/** @brief Check status of ROI. */
bool OverlapRegisteredImages::isRegionOfInterestBelowThresh( std::string &errMsg )
{
  int width = _minRect.width;
  int height = _minRect.height;
  if( width < ROI_THRESH ) {
    errMsg = "Registered images overlap region does not meet width threshold="
      + std::to_string(ROI_THRESH) + ", is " + std::to_string(width) + ".";
    return true;
  }
  else if( height < ROI_THRESH ) {
    errMsg = "Registered images overlap region does not meet height threshold="
      + std::to_string(ROI_THRESH) + ", is " + std::to_string(height) + ".";
    return true;
  }
  else
    return false;
}

/** @brief Check status of ROI. */
bool OverlapRegisteredImages::isRegionOfInterestEmpty()
{
  return _minRect.empty();
}

/**
 * @brief The structuring element (kernel) that was used to dilate the
 *  image that is the summed-overlap of the registered images.
 *
 * Useful for test/debug purposes.
 *
 * @return kernel type and size in print- format
 */
std::string OverlapRegisteredImages::getStructuringElementParams()
{
  return _dilationKernelParams.to_s();
}


/**
 * @brief Image used to calculate the common, ROI crop coordinates.
 *
 * @return vector of unsigned bytes
 */
std::vector<uint8_t> OverlapRegisteredImages::getPngBlob() const
{
  return _vecPngBlob;
}


/**
 * @brief Top-left, bottom-right corners of crop-region, width, height,
 *  and area.
 * 
 * @return string in print format
 */
std::string OverlapRegisteredImages::to_s()
{
  std::string s1{"OverlapRegisteredImages:\n"};
  s1 +=  " * Rect TopLeft: (" + std::to_string( _minRect.tl().x ) + ", "\
                              + std::to_string( _minRect.tl().y ) + ")\n";
  std::string s2{""};
  s2 =  " * Rect BotRight: (" + std::to_string( _minRect.br().x ) + ", "\
                              + std::to_string( _minRect.br().y ) + ")\n";

  std::string s3{" * Rect dimensions:\n"};
  s3 += "    width:  " + std::to_string(_minRect.width) + "\n";
  s3 += "    height: " + std::to_string(_minRect.height) + "\n";
  s3 += "    area:   " + std::to_string(_minRect.area()) + "\n";

  std::string s4{_dilationKernelParams.to_s()};

  return s1 + s2 + s3 + s4;
}

/**
 * @return current size and type for metadata generated-by and
 *  included-in the registration process */
std::string OverlapRegisteredImages::DilationKernelParams::to_s() const
{
  std::string s3{" * Sum binaries dilation parameters for kernel:\n"};
  s3 += "    size: "
    + std::to_string( size ) + "\n";

  // From opencv2/imgproc.hpp, line 230:
  std::string s4{"    type: "};
  if( type == cv::MORPH_RECT ) {
    s4 += std::to_string( type ) + " = cv::MORPH_RECT\n";
  }
  else if( type == cv::MORPH_CROSS ) {
    s4 += std::to_string( type ) + " = cv::MORPH_CROSS\n";
  }
  else if( type == cv::MORPH_ELLIPSE ) {
    s4 += std::to_string( type ) + " = cv::MORPH_ELLIPSE\n";
  }

  return s3 + s4;
}

}   // End namespace
