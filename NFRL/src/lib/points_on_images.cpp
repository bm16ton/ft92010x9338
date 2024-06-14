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
#include "points_on_image.h"
#include "points_on_images.h"

namespace NFRL {

/**
 * @brief Calculate the scale factor (ratio) between image-pair segments.
 * 
 * @param pairMoving points on moving image
 * @param pairFixed  points on fixed image
 */
PointsOnImages::PointsOnImages( NFRL::PointsOnImage &pairMoving,
                                NFRL::PointsOnImage &pairFixed )
  : _pairMoving(pairMoving), _pairFixed(pairFixed)
{
  _scaleFactor = _pairMoving.segmentLength / _pairFixed.segmentLength;
}

/**
 * @brief Scale factor based on length of segments.
 *
 * Scale factor is ratio of segment lengths on each image: (img1 / img2).
 * 
 * @return the scale factor
 */
double PointsOnImages::getScaleFactor() const
{
  return _scaleFactor;
}

/** @brief The points-pairs on the (same) image for both images.
 *
 * @return string of point-pairs on both images
 */
std::string PointsOnImages::to_s() const
{
  std::string out;
  std::string s1 = _pairMoving.to_s("moving");
  std::string s2 = _pairFixed.to_s("fixed");
  out = "  #1: " + s1 + "  #2: " + s2;
  return out;
}

}   // End namespace
