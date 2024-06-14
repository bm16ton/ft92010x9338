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
#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>


namespace NFRL {

/**
 * @brief To determine the region to crop each source image, the registered
 * images are overlaid by "summing" their binary renderings.
 *
 * This overlap region is the rectangle (ROI) that is used to crop source
 * images.  Therefore, final images are same size (the size of the crop-region).
 */
class OverlapRegisteredImages
{
private:
  /** @brief White pixel in grayscale image. */
  int const _max_BINARY_value = 255;

  /** @brief The minimum rectangle surrounding the *REGISTERED* moving image
   *   overlapping the fixed image. */
  cv::Rect _minRect;
  /** @brief Byte-stream of overlay region only. */
  std::vector<uint8_t> _vecPngBlob;

  /** @brief OpenCV support for image dilation. */
  struct DilationKernelParams {
    /** @brief [ MORPH_ELLIPSE | MORPH_CROSS | MORPH_RECT ]
     *
     * This type is set programmatically in source code; must recompile
     * to change. */
    int type{ cv::MORPH_RECT };
    /** @brief Size of the kernel, initialized to invalid value forces error
     *   if not updated.
     *
     *  This size is set programmatically in source code; must recompile
     *  to change. */
    int size{-1};

    /** @brief Returns current size and type for metadata generated-by and
     *  included-in the registration process. */
    std::string to_s() const;
  }
  /** @brief Container for size and type of the dilation kernel. */
  _dilationKernelParams;

  /** @brief Ensure that the ROI is valid. */
  bool isRegionOfInterestEmpty();
  bool isRegionOfInterestBelowThresh( std::string & );

public:

  void Init();
  void Copy( const OverlapRegisteredImages& );

  // Default constructor.
  OverlapRegisteredImages();

  // Copy constructor.
  OverlapRegisteredImages( const OverlapRegisteredImages& );

  /** @brief Full constructor used by NFRL. */
  OverlapRegisteredImages( cv::Mat, cv::Mat );
  virtual ~OverlapRegisteredImages() {}

  /** @brief Rectangle of overlap for cropping of source images. */
  cv::Rect getRegionOfInterest() const;
  /** @brief Top-left and bottom-right. */
  std::vector<std::string> getRegionOfInterestCorners() const;

  /** @brief Image used to calculate the common, ROI crop coordinates. */
  std::vector<uint8_t> getPngBlob() const;

  // Debug metadata
  std::string getStructuringElementParams();
  std::string to_s();
};

}   // End namespace
