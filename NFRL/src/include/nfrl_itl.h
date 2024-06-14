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

#include "nfrl_lib.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

/**
 * @brief Implements a "wrapper" to the NFRL "core" library that takes image
 *  data as OpenCV types in the API.
 *
 * The public class and all public methods exposed here reference the
 * corresponding public interfaces in the NFRL-core.
 * This wrapper prevents any would-be-required duplication of code in the event
 * of future source-code modifications (best practice).
 * However, the NFRL (library) user must reference the NFRL-core directly for
 * access to registration metadata and exception handling.
 * 
 * For example:
 * ```
 *   NFRL_ITL::Registrator *r2;
 *
 *   NFRL::Registrator::RegistrationMetadata rm2;
 *
 *   NFRL_ITL::printVersion();
 *
 *   catch( NFRL::Miscue &e ) {}
 * ```
 */
namespace NFRL_ITL {

/** @brief Wrapper API to support OpenCV. */
class Registrator
{
private:

  /** @brief Declare the pointer to NFRL core. */
  std::unique_ptr<NFRL::Registrator> _r2;

  /** @brief Byte-stream of the Moving image. */
  cv::Mat _imgMovingMat;
  /** @brief Byte-stream of the Fixed image. */
  cv::Mat _imgFixedMat;
  /** @brief 8 individual coordinates of the two registration pairs of points. */
  std::vector<int> &_correspondingPoints;
  /** @brief Each run of the registration process captures metadata for use
   *   by the caller. */
  std::vector<std::string> &_metadata;

public:
  // Default constructor.
  Registrator();

  /** @brief Full constructor used by NFRL with OpenCV API. */
  Registrator( cv::Mat &, cv::Mat &,
               std::vector<int> &, std::vector<std::string> & );
  virtual ~Registrator() {}   // smart-pointer precludes delete _r2; call

  void performRegistration();

  void getMetadata( NFRL::Registrator::RegistrationMetadata& );
  void getXmlMetadata( XmlMetadata& );

  cv::Mat getColorOverlaidRegisteredImages();
  cv::Mat getCroppedFixedImage();
  cv::Mat getCroppedRegisteredImage();
  cv::Mat getPaddedFixedImg();
  cv::Mat getPaddedRegisteredMovingImg();
  cv::Mat getPngBlob();

  /* Padding Size get functions */
  int getMovingPadSizeLeft();
  int getMovingPadSizeTop();
  int getFixedPadSizeLeft();
  int getFixedPadSizeTop();

  void saveCroppedRegisteredImageToDisk( std::string );
  void saveCroppedFixedImageToDisk( std::string );

};

/** @brief Wrapper method, returns the current versions of this
 *   NFRL software and OpenCV. */
std::string printVersion();


}   // END namespace
