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
#include "corresponding_points_pair.h"
#include "corresponding_points_pairs.h"

// #include <iostream>

namespace NFRL {

/** @brief Initialization function that resets both pairs of points to (0,0). */
void CorrespondingPointsPairs::Init()
{
  pair1 = NFRL::CorrespondingPointsPair( cv::Point2f(0, 0), cv::Point2f(0, 0) );
  pair2 = NFRL::CorrespondingPointsPair( cv::Point2f(0, 0), cv::Point2f(0, 0) );
}

/** @brief Supports copy-constructor. */
void CorrespondingPointsPairs::Copy( const CorrespondingPointsPairs& aCopy )
{
  pair1 = aCopy.pair1;
  pair2 = aCopy.pair2;
}

/** @brief Default constructor.  Calls Init(). */
CorrespondingPointsPairs::CorrespondingPointsPairs()
{
  Init();
}

/** @brief Copy constructor.  This is called when passing the object by value
 * as parameter to CorrespondingPointsPairs constructor.
 * 
 * @param aCopy object to be copied
 */
CorrespondingPointsPairs::CorrespondingPointsPairs(
    const CorrespondingPointsPairs& aCopy )
{
  Copy( aCopy );
}

/** @brief This pair of points are corresponding *across* images.
 *
 * @param pair1 as input into the registration process (method-call)
 * @param pair2 as input into the registration process (method-call)
 */
CorrespondingPointsPairs::CorrespondingPointsPairs(
    NFRL::CorrespondingPointsPair pair1, NFRL::CorrespondingPointsPair pair2 )
  : pair1(pair1), pair2(pair2) {}

/** @brief `#1: (x1,y1) * (x2,y2)  #2: (x3,y3) * (x4,y4)`
 *
 * @return string of points-pairs: `#1: (x1,y1) * (x2,y2)  #2: (x3,y3) * (x4,y4)`
 */
std::string CorrespondingPointsPairs::to_s() const
{
  std::string out{};
  std::string s1 = pair1.to_s();
  std::string s2 = pair2.to_s();

  out = "  #1: " + s1 + "  #2: " + s2;
  return out;
}

}   // End namespace
