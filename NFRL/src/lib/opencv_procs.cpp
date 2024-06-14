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

#include <opencv2/opencv.hpp>

namespace CVops {

/**
 * @brief Uses OpenCV adaptiveThreshold() to convert a grayscale image to binary.
 *
 * That is, the resulting image has pixels values of either 0 (black) or
 * 255 (white).
 *
 * The threshold value (parameter) is compared against each pixel value in the
 * source image; it (the threshold value) is calculated for smaller regions and
 * therefore, there will be different threshold values for different regions.
 *
 * Binarized image is inverted.
 * ```
 * if source-pixel < threshold
 *   destination-pixel = maxBinaryValue
 * else
 *   destination-pixel = 0
 * ```
 *
 * @param imgIn IN image to binarize
 * @param imgBinary OUT resulting binarized image
 * @param maxBinaryValue non-zero value assigned to the pixels for which the
 *                       condition is satisfied
 */
void binarize_image_via_adaptive_threshold( const cv::Mat &imgIn,
                                            cv::Mat &imgBinary,
                                            const int maxBinaryValue )
{
  int blockSize = 5;
  double C{0.0};
  cv::adaptiveThreshold( imgIn, imgBinary, maxBinaryValue,
                         cv::ADAPTIVE_THRESH_MEAN_C,
                         cv::THRESH_BINARY_INV,
                         blockSize, C );
}

/**
 * @brief Uses OpenCV threshold() to convert a grayscale image to binary.
 *
 * That is, the resulting image has pixels values of either 0 (black)
 * or 255 (white).
 *
 * The Otsu flag tells the threshold function to use Otsu’s method in
 * automatically determining a global threshold value (and thus the “hard-coded”
 * threshold value parameter is ignored).
 *
 * @param imgIn IN image to binarize
 * @param imgBinary OUT resulting binarized image
 * @param maxBinaryValue 255 for grayscale
 */
void binarize_image_via_otsu_threshold( const cv::Mat &imgIn,
                                        cv::Mat &imgBinary,
                                        const int &maxBinaryValue )
{
  cv::threshold( imgIn, imgBinary, 0, maxBinaryValue,
                 cv::THRESH_BINARY | cv::THRESH_OTSU );
}

/**
 * @brief Uses OpenCV threshold() to convert a grayscale image to binary.
 *
 * That is, the resulting image has pixels values of either 0 (black)
 * or 255 (white).
 *
 * The threshold value (parameter) is compared against each pixel value in the
 * source image.  If the pixel value is less than threshold, set the
 * corresponding pixel in the output image to zero.  If greater, set to
 * maxBinaryValue.
 *
 * ```
 * if source-pixel < threshold
 *   destination-pixel = 0
 * else
 *   destination-pixel = maxBinaryValue
 * ```
 *
 * @param imgIn IN image to binarize
 * @param imgBinary OUT resulting binarized image
 * @param threshold 250 seems to work well
 * @param maxBinaryValue 255 for grayscale
 */
void binarize_image_via_threshold( const cv::Mat &imgIn, cv::Mat &imgBinary,
                                   const int &threshold,
                                   const int &maxBinaryValue )
{
  cv::threshold( imgIn, imgBinary, threshold, maxBinaryValue,
                 cv::THRESH_BINARY );
}


/**
 * @brief Convert cv::Mat type to 2D array of vectors.
 *
 * @param matrix to convert
 *
 * @return Rotate2D
 */
Rotate2D cast_rotation_matrix( const cv::Mat &matrix )
{
  int tRows{matrix.rows};
  int tCols{matrix.cols};
  Rotate2D out;
  out.resize( tRows, std::vector<float>(tCols,0) );
  for( int i=0; i<tRows; i++ )
  {
    for( int j=0; j<tCols; j++ )
    {
      out[i][j] = static_cast<float>( matrix.at<double>(i,j) );
    }
  }
  return out;
}


/**
 * @brief Convert cv::Mat type to 2D array of vectors.
 *
 * @param matrix to convert
 *
 * @return Translate2D
 */
Translate2D cast_translation_matrix( const cv::Mat &matrix )
{
  int tRows{matrix.rows};
  int tCols{matrix.cols};
  Translate2D out;
  out.resize( tRows, std::vector<int>(tCols,0) );
  for( int i=0; i<tRows; i++ )
  {
    for( int j=0; j<tCols; j++ )
    {
      out[i][j] = static_cast<int>( matrix.at<float>(i,j) );
    }
  }
  return out;
}


/**
 * @brief Uses OpenCV crop_image() to crop an image given a cv::Rect object
 *  that represents the rectangular region to crop.
 *
 * @param img to crop
 * @param roi region of interest (the crop region)
 *
 * @return matrix that is the cropped image
 */
cv::Mat crop_image( const cv::Mat &img, const cv::Rect &roi)
{
  cv::Mat cropped = img( roi );
  return cropped;
}


/**
 * @brief Uses OpenCV dilate() to dilate/thicken the image.
 *
 * Generate the structuring element that is the kernel used during the dilate
 * operation.
 *
 * cv::dilate signature:
 * ```
 * void dilate( InputArray src, OutputArray dst, InputArray kernel,
 *              Point anchor=Point(-1,-1), int iterations=1,
 *              int borderType=BORDER_CONSTANT,
 *              const Scalar& borderValue=morphologyDefaultBorderValue() )
 * note: for kernel, cv::Mat() is 3x3 by default
 * ```
 *
 * From opencv2/imgproc.hpp, line 230:
 * ```
 * enum MorphShapes {
 *     MORPH_RECT    = 0,
 *     MORPH_CROSS   = 1,
 *     MORPH_ELLIPSE = 2
 * };
 * ```
 *
 * @param img IN image to dilate
 * @param imgDilate OUT resulting dilated image
 * @param knlSize IN size of structuring-element kernel
 * @param knlType IN type of structuring-element kernel
 */
void image_dilate( const cv::Mat &img, cv::Mat &imgDilate,
                   const int &knlSize, const int &knlType )
{
  cv::Mat element = cv::getStructuringElement( knlType,
                    cv::Size( 2*knlSize + 1, 2*knlSize+1 ),
                    cv::Point( knlSize, knlSize ) );
  cv::dilate( img, imgDilate, element );
}


/**
 * @brief "Sum" the two image erosions to calculate the overlap.
 *
 * This is done pixel-by-pixel.  If both pixels are black (0), then set the
 * pixel at the coordinates to 0.  Otherwise set to white (255).
 * Obviously, both images must have the same width-by-height dimensions.
 * 
 * @param img1 IN addend
 * @param img2 IN addend
 * @param imgSum OUT the sum of the two binary images
 */
void sum_two_binary_images( const cv::Mat &img1, const cv::Mat &img2,
                            cv::Mat &imgSum )
{
  for(int i=0; i < img1.rows; i++) {
    for(int j=0; j < img1.cols; j++) {
      if( (img1.at<uint8_t>(i,j) == 0 ) && (img2.at<uint8_t>(i,j) == 0 ) ) {
        imgSum.at<uint8_t>(i,j) = 0;
      }
      else {
        imgSum.at<uint8_t>(i,j) = 255;
      }
    }
  }
}


/**
 * @brief Support for logging.
 *
 * @param matrix to convert to single string
 *
 * @return single string including new-lines to nicely format the matrix
 */
std::string rotation_matrix_to_s( const cv::Mat &matrix )
{
  int tRows{matrix.rows};
  int tCols{matrix.cols};
  std::string sTmp{};
  std::string sMatrix{"["};
  for( int i=0; i<tRows; i++ )
  {
    for( int j=0; j<tCols; j++ )
    {
      sTmp = std::to_string( matrix.at<double>(i,j) );
      sMatrix.append(sTmp);
      if( j < tCols-1 )
        sMatrix.append(", ");
    }
    if( i < tRows-1 )
      sMatrix.append(":\n ");
  }
  sMatrix.append("]");
  return sMatrix;
}


/**
 * @brief Support for logging.
 *
 * @param matrix to convert to single string
 *
 * @return single string including new-lines to nicely format the matrix
 */
std::string translation_matrix_to_s( const cv::Mat &matrix )
{
  int tRows{matrix.rows};
  int tCols{matrix.cols};
  std::string sTmp{};
  std::string sMatrix{"["};
  for( int i=0; i<tRows; i++ )
  {
    for( int j=0; j<tCols; j++ )
    {
      sTmp = std::to_string( static_cast<int>( matrix.at<float>(i,j) ) );
      sMatrix.append(sTmp);
      if( j < tCols-1 )
        sMatrix.append(", ");
    }
    if( i < tRows-1 )
      sMatrix.append(":\n ");
  }
  sMatrix.append("]");
  return sMatrix;
}

}   // END namespace
