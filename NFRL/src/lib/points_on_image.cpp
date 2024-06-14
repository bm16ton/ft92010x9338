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

namespace NFRL {

/** @brief Initialization function that resets all values. */
void PointsOnImage::Init() {
  _pointOne = cv::Point2f(0, 0);
  _pointTwo = cv::Point2f(0, 0);
  _slope = 0;
  _sideX = -1;
  _sideY = -1;
}

/** @brief Supports copy-constructor. */
void PointsOnImage::Copy( const PointsOnImage& aCopy )
{
  _pointOne = aCopy._pointOne;
  _pointTwo = aCopy._pointTwo;
}

/** @brief Default constructor. Calls Init(). */
PointsOnImage::PointsOnImage()
{
  Init();
}

/** @brief Copy constructor.
 *
 * This is called when passing the object by value as parameter to
 * PointsOnImages constructor.
 */
PointsOnImage::PointsOnImage( const PointsOnImage& aCopy )
{
  Copy( aCopy );
}

/**
 * @brief Calculate the angle between the segment defined by the two points
 *  and the horizontal.
 *
 * Since image origin (0, 0) is the top-left corner, all point coordinates
 * are positive, and therefore x increases to the right and y increases down.
 *
 * Since OpenCV defines the origin of an image to be the top-left corner, the
 * angle that this function calculates must be the additive inverse of the
 * actual calculation.
 *
 * For a segment that is not vertical, sideX is not equal to 0 so its slope
 * is calculated.  Then, to compensate for Y-axis positive down, check sign
 * of sideX and slope, and take the additive inverse if necessary.
 *
 * For example, if the segment is vertical pointing up, then in classic
 * Cartesian context, the angle from horizontal is +90 degrees.  However,
 * since the Y-axis increases down, this function returns -90 degrees.
 * 
 * Range of angleDegrees: [0,360].
 * 
 * @param pointOne (x,y) first-selected point on image
 * @param pointTwo (x,y) second-selected point on image
 */
PointsOnImage::PointsOnImage( cv::Point2f pointOne, cv::Point2f pointTwo )
{
  double pi = 2 * acos(0.0);
  _pointOne = pointOne;
  _pointTwo = pointTwo;
  _slope = 0;   // initalize in case slope is not calculated

  _sideX = _pointTwo.x - _pointOne.x;
  _sideY = _pointTwo.y - _pointOne.y;
  segmentLength = sqrt( std::pow(_sideX, 2) + std::pow(_sideY, 2) ) ;

  int sX{static_cast<int>( _sideX )};

  if(( sX == 0) && (pointTwo.y > pointOne.y)) {  // vertical ray down
    angleDegrees = -90.0;
  }
  else if(( sX == 0) && (pointTwo.y <= pointOne.y)) {   // vertical ray up
    angleDegrees = 90.0;
  }
  else {
    _slope = _sideY / _sideX;
    _slope = -_slope;    // image origin is located at top-left
    angleDegrees = std::acos( _sideX / segmentLength ) * 180.0 / pi;

    if( ( _sideX > 0.0 ) && ( _slope < 0 ) ) {   // classic Cartesian quad IV
      angleDegrees *= -1.0;
    }
    else if( ( _sideX < 0.0 ) && ( _slope > 0 ) ) {   // classic Cartesian quad II
      angleDegrees *= -1.0;
    }
  }
}

/**
 * @brief All metadata per registration.
 *
 * @param kind [ moving | fixed ] image
 *
 * @return single string (with trailing `\n`) of all relevant info
 */
std::string PointsOnImage::to_s( const std::string &kind ) const
{
  std::string str0{""}, str1{""}, str2{""}, str3{""};
  str0 = "(" + std::to_string(_pointOne.x) + ", "
       + std::to_string(_pointOne.y) + ") * (" + std::to_string(_pointTwo.x)
       + ", " + std::to_string(_pointTwo.y) + ")\n";
  str1 = kind + " _sideX: " + std::to_string(_sideX)
       + ", _sideY: " + std::to_string(_sideY) + ", segmentLength: "
       + std::to_string(segmentLength) + "\n";
  str2 = kind + " SLOPE: " + std::to_string(_slope) + "\n";
  str3 = kind + " Angle from horizontal: " + std::to_string(angleDegrees)
       + " degrees\n";
  return str0 + str1 + str2 + str3;
}

/** @brief The points-pair on the (same) image.
 *
 * @return points on image in a vector container
 */
std::vector<cv::Point2f> PointsOnImage::getVectorOfPoints() const
{
  std::vector<cv::Point2f> v;
  v.push_back( _pointOne );
  v.push_back( _pointTwo );
  return v;
}

}   // End namespace
