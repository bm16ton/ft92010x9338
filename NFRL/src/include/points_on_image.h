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

#include <opencv2/core/core.hpp>

namespace NFRL {

/**
 * @brief For each manual registration using two (2) pairs of corresponding
 *  control.
 *
 * Points *across* images, each image *itself* "contains" two control points:
 * the "points on image" pair.
 *
 * There are two issues of interest:
 * 1. the length of the segment (in pixels)
 * 2. the angle of the segment with respect to the horizontal.
 * 
 * The segment length is used to calculate the "scale factor" for the sample
 * rate of the image where the scale factor is the ratio of two line-segment
 * lengths.  In theory, the basis segment for the ratio is that taken from
 * a device that is known to capture images at 500 pixels per inch (500PPI).
 * 
 * The angle is used to calculate the amount of rigid rotation required to
 * register two images.
 */
class PointsOnImage
{

public:

  void Init();
  void Copy( const PointsOnImage& );

  // Default constructor.
  PointsOnImage();

  // Copy constructor.
  PointsOnImage( const PointsOnImage& );

  /** @brief Full constructor used by NFRL. */
  PointsOnImage( cv::Point2f, cv::Point2f );
  virtual ~PointsOnImage() {}

  /** @brief Angle of the segment from the horizontal where 0 degrees is the
   *   x-axis of the Cartesian coordinate system, i.e., the "ray" between the
   *   first and fourth quadrants. */
  double angleDegrees;
  /** @brief Euclidean distance between the two points, i.e., the "segment". */
  double segmentLength;

  std::string to_s( const std::string& ) const;
  std::vector<cv::Point2f> getVectorOfPoints() const;

private:
  /** @brief Of the segment */
  cv::Point2f _pointOne;
  /** @brief Of the segment */
  cv::Point2f _pointTwo;
  /** @brief Slope of the segment (where segment = Euclidean hypotenuse) */
  float _slope;
  /** @brief Of right-triangle */
  float _sideX;
  /** @brief Of right-triangle */
  float _sideY;

};

}   // End namespace
