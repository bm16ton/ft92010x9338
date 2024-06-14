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

#include "exceptions.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

/** @brief Object of this type is used strictly for registration metadata. */
typedef std::vector<std::vector<float>> Rotate2D;
/** @brief Object of this type is used strictly for registration metadata. */
typedef std::vector<std::vector<int>> Translate2D;

/** @brief Registration metadata in XML format.
 *   Each string in the vector is a correctly-formed XML text string.  The
 *   object of this type must be utilized in its entirety to obtain a complete
 *   XML doc. */
typedef std::vector<std::string> XmlMetadata;

#define NFRL_VERSION "0.1.0"


#ifdef USE_OPENCV
  /**
   * @brief Implements the NFRL "core" library that takes image data as
   *  byte-streams in the API.
   *
   * The NFRL (library) user references the NFRL-core directly for access
   * to registration metadata and exception handling.
   */
  namespace NFRL {
#else
  namespace NFRL_ITL {
#endif

/** @brief The current versions of this NFRL software and OpenCV. */
std::string printVersion();


/**
 * @brief Instantiate this class and call the performRegistration() function
 * to perform the entire registration process on a single pair of images.
 */
class Registrator
{
private:
  /** @brief Byte-stream of the Moving image. */
  std::vector<uint8_t> _imgMoving;
  /** @brief Byte-stream of the Fixed image. */
  std::vector<uint8_t> _imgFixed;

  /** @brief 8 individual coordinates of the two registration pairs of points.
   *
   * In order: [(x1,y1) (x2,y2) (x3,y3) (x4,y4)]
   */
  std::vector<int> &_correspondingPoints;

  /** @brief Each run of the registration process captures metadata for use
   *   by the caller. */
  std::vector<std::string> &_metadata;

  /** @brief Byte-stream of the registered, cropped, Moving image. */
  std::vector<uint8_t> _vecCroppedRegisteredImage;
  /** @brief Byte-stream of the registered, cropped, Fixed image. */
  std::vector<uint8_t> _vecCroppedFixedImage;
  /** @brief Byte-stream of the padded, overlaid, registered images
   *   as a single image. */
  std::vector<uint8_t> _vecColorOverlaidRegisteredImages;
  /** @brief Byte-stream of padded, registered, Fixed image, grayscale. */
  std::vector<uint8_t> _vecPaddedFixedImg;
  /** @brief Byte-stream of padded, registered, Moving image, grayscale. */
  std::vector<uint8_t> _vecPaddedRegisteredMovingImg;

  // Registration-process imagery
  /** Byte-stream of padded, yet to be registered, Moving image. */
  // std::vector<uint8_t> _vecPaddedUnregisteredMovingImg;
  /** Byte-stream of padded, yet to be registered, Fixed image. */
  // std::vector<uint8_t> _vecColorPaddedUnregisteredFixedImg;

  // Debug imagery
  /** @brief Byte-stream of blob of overlay region only. */
  std::vector<uint8_t> _vecPngBlob;

  /** @brief Supports padding of source images prior to registration. */
  struct PaddingDifferential
  {
    /** @brief Margin from top of source image to padded edge. */
    int top;
    /** @brief Margin from bottom of source image to padded edge. */
    int bot;
    /** @brief Margin from left of source image to padded edge. */
    int left;
    /** @brief Margin from right of source image to padded edge. */
    int right;

    /** @brief Constructor sets all pad values to zero. */
    PaddingDifferential()
    {
      top = 0; bot = 0; left = 0; right = 0;
    }
    /** @brief Reset all pad values to zero. */
    void reset()
    {
      top = 0; bot = 0; left = 0; right = 0;
    }
  }
  /** @brief Container for padding values (all 4 sides) for the Moving image. */
  _padDiffMoving,
  /** @brief Container for padding values (all 4 sides) for the Fixed image. */ 
  _padDiffFixed;


  /** @brief Support for registration metadata capture. */
  struct Point
  {
    /** @brief x-coord of a point */
    int x;
    /** @brief y-coord of a point */
    int y;
    // Print the coords as a comma-separated string.
    std::string to_s() const;
    // Convert the coords to vector of ints: in order x, y.
    void to_v( std::vector<int> & );
  };


  /** @brief Support for registration metadata capture. */
  struct ImageSize
  {
    int width{-1};
    int height{-1};

    std::string getWidth() { return std::to_string( width ); }
    std::string getHeight() { return std::to_string( height ); }
    void set( int w, int h ) { width = w; height = h; }
    // WxH as string
    std::string to_s() const;
  };


  /** @brief Support for registration metadata scale factor (sf) calculation. */
  enum ScaleFactorDirection
  {
    /** sf = img1/img2 (default) */
    img1_to_img2 = 1,
    /** sf = img2/img1 */
    img2_to_img1
  };


public:
  /**
   * @brief This struct is used to capture registration metadata calculated
   *  each time a pair of images is registered.
   *
   * The user of this NFRL library has access to the metadata per this custom
   * type.  Also, the metadata is injected into XML that is output as a vector
   * of strings (see Registrator::getXmlMetadata( XmlMetadata ) ).
   */
  struct RegistrationMetadata
  {
    // ----- Translation -----
    /** @brief Translation in x-direction. */
    int tx{0};
    /** @brief Translation in y-direction. */
    int ty{0};

    /** @brief Loaded using CVops::cast_translation_matrix() method. */
    Translate2D translMatrix;

    // Return strings for logging support
    std::vector<std::string> getTranslationTransform();

    // ----- Rotation -----
    /** @brief Angle between segments on each image. */
    double angleDiffDegrees{0.0};

    /**
     * @brief Center of rotation, by design, is the first control point
     *  on the Fixed image.
     *
     * This is the point to which the Moving image is translated.
     */
    Point centerRot;

    /**
     * @brief Retrieve the center of rotation point as a vector.
     *
     * This is essentially the point on the Fixed image to which the Moving
     * image was rigidly translated.  The registration is completed by then
     * rotating the Moving image about this point.
     * 
     * @param center IN OUT vector with two elements: in order x,y
     */
    void centerOfRotation( std::vector<int> &center ) { centerRot.to_v( center ); }

    /** @brief Loaded using CVops::cast_rotation_matrix() method. */
    Rotate2D rotMatrix;

    // Return strings for logging support
    std::vector<std::string> getRotationTransform();

    // ----- Control points -----
    /**
     * @brief Four points of the registered images after the registration, two
     * on each image.
     *
     * The key of the map-container is "ptX" where X is the point number.
     * The Euclidean distances are those between the control points.
     */
    struct ControlPoints
    {
      /** @brief There are four total control-point, i.e., two pairs.
       *
       *  Each of the four control points numbered, e.g., point #1 = (123, 456).
       */
      std::map<std::string, Point> point;
      /**
       * @brief Distances between transformed points (post registration).
       *
       * The unconstrained pair is that used for translation.
       * The constrained pair is that used for rotation.
       */
      struct EuclideanDistance
      {
        /** @brief Distance between constrained pair of points (across images,
         *   the two points used for rotation). */
        double constrained{0.0};
        /** @brief Distance between unconstrained pair of points (across images,
         *   the two points used for translation). */
        double unconstrained{0.0};

        // Return string for logging support
        std::string to_s_constrained() const;
        // Return string for logging support
        std::string to_s_unconstrained() const;
      }
      /**
       * @brief Euclidean distances between transformed points (post registration).
       */
      euclideanDistance;

      // Get the point as string in point-format
      std::string getControlPoint( short );

      // Set the point coordinates and push into map of control points.
      void setControlPoint( short, int, int );
    }
    /** @brief Container for four points of the registered images after the
     *   registration. */
    controlPoints;

    // ----- Scale factor
    /** @brief Ratio of segment lengths and "direction" of registration. */
    struct ScaleFactor
    {
      /** @brief Ratio of segment lengths. */
      double value;
      /** @brief Which image segment length is in the denominator of ratio,
       *   i.e., image1 or image2. */
      ScaleFactorDirection direction;

      /** @brief "Direction" of the ratio. */
      std::string getScaleFactorDirection(void) const;
    }
    /** @brief Container for ratio of segment lengths and 'direction'
     *   of registration.
     */
    scaleFactor;

    // ----- Report image sizes to caller
    /** @brief W x H */
    ImageSize srcMovingImgSize;
    /** @brief W x H */
    ImageSize srcFixedImgSize;
    /** @brief W x H */
    ImageSize paddedImgSize;
    /** @brief W x H */
    ImageSize registeredImgSize;

    // ----- Convert source image to grayscale
    /** @brief Support for registration metadata.
     *
     * Notification that a source image has been converted to
     * grayscale in order to continue the registration process.
     */
    struct ConvertToGrayscale
    {
      /** @brief Set to true if image was converted to grayscale.*/
      bool img1{false};
      /** @brief Set to true if image was converted to grayscale. */
      bool img2{false};

      // Check if either image was converted.
      bool any(void) const; // {

      /** @brief Support registration metadata XML logging. */
      std::string img1_to_s() const;
      /** @brief Support registration metadata XML logging. */
      std::string img2_to_s() const;
    }
    /** @brief Container for notification for caller that a source image has
     *   been converted to grayscale. */
    convertToGrayscale;

    /** @brief Top-left and bottom-right corner's (pixel locations)
     *   of ROI rectangle. */
    std::vector<std::string> overlapROICorners;

  }
  /** @brief Container that captures registration metadata during registration. */
  registrationMetadata;


public:

  void Init();
  void Copy( const Registrator& );

  // Default constructor.
  Registrator();

  // Copy constructor.
  Registrator( const Registrator& );

  /** @brief Full constructor used by NFRL.
   *
   * The caller instantiates this class and calls the performRegistration()
   * function to perform the entire registration process on a single pair of
   * images where the registration points are corresponding control points as
   * determined by the caller. */
  Registrator( std::vector<uint8_t>, std::vector<uint8_t>,
               std::vector<int> &, std::vector<std::string> & );
  virtual ~Registrator() {}

  /** @brief Call this function to register two images.
   *
   * Imagery, control-points, and registration metadata containers are
   * initialized in full constructor. */
  void performRegistration();

  std::vector<uint8_t> getColorOverlaidRegisteredImages();
  std::vector<uint8_t> getCroppedRegisteredImage();
  std::vector<uint8_t> getCroppedFixedImage();
  std::vector<uint8_t> getPaddedFixedImg();
  std::vector<uint8_t> getPaddedRegisteredMovingImg();
  std::vector<uint8_t> getPngBlob();


  // Get by reference.
  void getMetadata( RegistrationMetadata& ) const;

  // Builds the XML nodes and inserts relevant registration metadata.
  void getXmlMetadata( XmlMetadata& );

  // The registered image is located in memory.
  void saveCroppedRegisteredImageToDisk( const std::string path ) const;

  // The registered image is located in memory.
  void saveCroppedFixedImageToDisk( const std::string path ) const;

  /** @brief Padding object for Moving image.
   *
   * @return current padding object for Moving image
   */
  PaddingDifferential getPadDiffMoving() const
  {
      return _padDiffMoving;
  }

  /** @brief Padding object for Fixed image.
   *
   * @return current padding object for Fixed image
   */
  PaddingDifferential getPadDiffFixed() const
  {
      return _padDiffFixed;
  }

private:
  void buildXmlTagline( XmlMetadata&, std::string );
  void buildXmlTagline( XmlMetadata&, std::string, std::string );

};

}   // END namespace
