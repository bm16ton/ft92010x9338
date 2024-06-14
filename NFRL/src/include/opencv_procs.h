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

/**
 * @brief Friendly interfaces to OpenCV methods.
*/
namespace CVops {

void binarize_image_via_adaptive_threshold( const cv::Mat&, cv::Mat &, const int = 1 );
void binarize_image_via_otsu_threshold( const cv::Mat&, cv::Mat &, const int& );
void binarize_image_via_threshold( const cv::Mat&, cv::Mat&, const int&, const int& );
cv::Mat crop_image( const cv::Mat&, const cv::Rect& );
void image_dilate( const cv::Mat&, cv::Mat&, const int&, const int& );
void sum_two_binary_images( const cv::Mat&, const cv::Mat&, cv::Mat& );

Rotate2D cast_rotation_matrix( const cv::Mat& );
Translate2D cast_translation_matrix( const cv::Mat& );
std::string rotation_matrix_to_s( const cv::Mat& );
std::string translation_matrix_to_s( const cv::Mat& );

}   // END namespace
