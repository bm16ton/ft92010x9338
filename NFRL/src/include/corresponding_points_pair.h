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
 * @brief Objects of this type contain a pair of points *across* images.
 *
 * In other words, one point of the pair is in the Moving image and the other
 * point is in the Fixed image.
 */
struct CorrespondingPointsPair final
{
  void Init();
  void Copy( const CorrespondingPointsPair& );

  // Default constructor.
  CorrespondingPointsPair();

  // Copy constructor.
  CorrespondingPointsPair( const CorrespondingPointsPair& );

  // Full constructor.
  CorrespondingPointsPair( cv::Point2f, cv::Point2f );
  ~CorrespondingPointsPair() {}

  /** @brief The point on the Moving image. */
  cv::Point2f movingPt;
  /** @brief The point on the Fixed image. */
  cv::Point2f fixedPt;

  double distance() const;
  std::string to_s() const;
};

}   // End namespace
