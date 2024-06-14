![nist-logo](doc/img/f_nist-logo-brand-2c_512wide.png)

The **NFRL (NIST Fingerprint-Image Registration Library)** builds and runs on Windows, Linux, and MacOS.

**NFRL** is written in C++ and built into a software library.  This affords users to write their own using-application.
This library is used by **NFRaCT** (www.nist.gov, search `nfract`), a cross-platform GUI application that registers
a pair of fingerprint images.

All computer-system requirements to build and store **NFRL** are fairly minimal and are easily satisfied by modern
hardware and software tools.  All software dependencies and tools used to compile source code and implement functionality
are available at no cost (free).

Across all operating system platforms, C++ 11 is required to compile.

**NFRL** depends on the OpenCV library (opencv.org) for image processing support; see section *Dependencies* below.


# Overview
**NFRL** registers two fingerprint images based on two pairs of corresponding control-points. It uses these pairs
of pixel locations within the images to translate and rotate the Moving image to the Fixed image.

Each image used in a registration process may be referred-to using multiple terms: moving or source, and, target, fixed,
or sensed.  For **NFRL**, the Fixed image remains fixed in 2-dimensional space and the Moving image is *moved* to align,
that is to register, with the Fixed image.

Runtime configuration parameters include:

* moving and fixed image data in 8-bits-per-pixel grayscale (preferable but not required)
* two-pairs of corresponding control points (pixel coordinates).

The fingerprint-image rigid-registration process is performed in two steps:

1. Translation of the Moving image to the Fixed image using the "first" pair of control-points (the constrained pair)
2. Rotation of the Moving image around the Fixed image control-point (the translation "target" location) based on the
angle-difference determined by the "second" pair of control-points (the *un*-constrained pair).

Both final images, a few interim images, and registration metadata generated during the registration process
are made available to the using software:

* Final registered Moving image
* Final registered Fixed image
* Registered, padded, overlaid image (colorized)
* Registered, padded, Moving image (grayscale)
* Padded, Fixed image (grayscale)
* Summed, registered, dilated overlaid image (the "blob")
* Process metadata available in both text and XML format.

The two Final images are registered.  They are cropped to the region-of-interest that is the smallest area of "overlap"
per the registration.  Therefore, these two images have identical width and height which enables analysis using metrics
like PSNR (Peak Signal to Noise Ratio); see also **NFRaCT**.

For the available images that are padded, the Fixed image is not translated or rotated.  However, it is
"registered" in the sense that the Moving image has been translated and rotated.  This is why the 'Padded, Fixed
image (grayscale)' (in the list of images above) is not referenced as "registered."

## Source Image Check
If either or both of the source images are not 8-bit grayscale, **NFRL** will convert them and attempt the registration.
Indication that an image has been converted (or not) is available as part of the registration metadata.

## Image Padding
To ensure the final, registered, Moving image does not have any portion of the ridge structure cut off, both images are
padded prior to the registration (process). Each image is padded with white pixels on all 4 sides.

The padding on the left and top MUST be the same for both images, and it follows that the right and bottom (values) fall
where they may to ensure padded images are the same size.

The *target* (padded) image size is based on the sizes of the input images; this *target* WxH is then used to calculate
the padding for both images resulting in two padded images that are the same size with the ridge structure centered.

Therefore, any ODD row or column are *flushed-out* at the right and bottom.  Subsequently, the left and top padding
(values) are used to *back-out* the padding for any/all registration calculations.

## Rigid Registration
Registration is performed in two steps: translation and then rotation.

### Translation
First, the translation matrix is calculated and is input into an algorithm that performs the translation of the padded
Moving image to the padded Fixed image.  The translated output image is the input image for rotation.

The pair of control-points used for translation (only) is denoted as "constrained" because these points have the same
coordinates after translation.  It follows that the Euclidean distance between them is zero.  

This "new origin" after translation is the constrained control point of the Fixed image (the translation "target"
location) plus the pixel location offsets (x,y) due to padding.

### Rotation
Second, the rotation matrix is calculated and is input into an algorithm that performs the rotation of the Moving image
that has been translated.

The angle from the horizontal for each line-segment within each image is calculated.  Note that each angle's value
is the addative inverse of its stardard Cartesian counterpart because OpenCV specifies the top-left corner of an image
as the origin.

The difference between these two angles is the degrees of rotation (-360, 360).  If the angle difference is negative,
the rotation is clockwise; if positive, rotation is counterclockwise.

The pair of control-points used for rotation (only) is denoted as "unconstrained" because these points are
"dragged along" with the translation.

The padded, translated Moving image is rotated around the corresponding, constrained control-point of the padded Fixed
image, that is, the point to which to Moving image has been translated.  That is, the images are now **registered**.

The padded, registered images (grayscale, Fixed and Moving) and the overlapped image (colorized) are made available
to the using-software for viewing to check for accuracy.

## Final Registered Images
The images are now registered, but they must be cropped.  The area to crop is the minimum area
(ROI - region of interest) that is common to both images.

The Otsu method for threshold is used to binarize source images in support of crop-region calculation.
Each registered image is binarized and the binary images are *summed* into a new image (array).  This *sum* is done
pixel-by-pixel; if both pixels are black (0), then set the pixel at the coordinates to 0.  Otherwise set to white (255).
The *summed* image is used to calculate the ROI crop area rectangle.

The final, cropped, registered images may be saved to disk (via function call).

If the area of overlap (that is the crop-region) does not meet a minimum width or height threshold, NFRL will throw
an exception.

## Registration Metadata
During the registration process, **NFRL** captures relevant data for further analysis, for example, translation and
rotation matrices, padded image size, point-selection coordinates, rotation angle, resultant-registration
control-points-pairs distances, and scale factor.  This metadata is available through function calls to retrieve the
metadata in plain-text and XML format.  The XML "Russian doll" schema is located in the `./doc` subdir.

## Thrown Exceptions
### Registrator Constructor
* throw NFRL::Miscue( "moving img buffer is empty" );
* throw NFRL::Miscue( "fixed img buffer is empty" );

### performRegistration() Function Call
* throw NFRL::Miscue( "Corresponding points count == {X}, should be 8" );
* throw NFRL::Miscue( "Moving image control-points identical, cannot continue" );
* throw NFRL::Miscue( "Fixed image control-points identical, cannot continue" );
* throw NFRL::Miscue( "OpenCV cannot decode image:" );
* throw NFRL::Miscue( "OpenCV cannot pad image:" );
* throw NFRL::Miscue( "Padded images not same size" );
* throw NFRL::Miscue( "OpenCV cannot colorize padded, fixed image:" );
* throw NFRL::Miscue( "OpenCV cannot perform translation:" );
* throw NFRL::Miscue( "OpenCV cannot perform rotation:" );
* throw NFRL::Miscue( "OpenCV cannot colorize padded-translated-rotated image:" );
* throw NFRL::Miscue( "OpenCV cannot merge overlaid images:" );
* throw NFRL::Miscue( "OpenCV cannot crop or save final images:" );

### saveCroppedRegisteredImageToDisk() Function Call
* throw NFRL::Miscue( "OpenCV cannot save image: '{path}'" );

### saveCroppedFixedImageToDisk() Function Call
* throw NFRL::Miscue( "OpenCV cannot save image: '{path}'" );

### OverlapRegisteredImages Constructor
* throw NFRL::Miscue( "OverlapRegisteredImages, cannot calc image-crop ROI:" );
* throw NFRL::Miscue( "Registered images overlap region is empty." );
* throw NFRL::Miscue( "Registered images overlap region does not meet width threshold=" );
* throw NFRL::Miscue( "Registered images overlap region does not meet height threshold=" );

# Dependencies
**NFRL** was tested with OpenCV 3.4.11 and 4.5.0.  It is recommended that the latest version of OpenCV be used.

OpenCV is used to support the **NFRL** algorithms.  Before proceeding, ensure that you have the necessary packages
installed.

All text in this README regarding system-environment and build-instructions are meant only as a guide.  Development
environments vary for many reasons, therefore, the instructions herein are *in general* and will probably require some
modifications.

## Local Libraries
Note that the supporting CMake files reflect the methods used to "install" OpenCV on Windows and Linux as "local"
libraries:

* Windows pre-built extracted to local file-system
* Linux built from source.

Because libraries are local, the development environment specifics (via system ENV variables) must be specified for
CMake (description below).

## Windows Tools and OpenCV Installations
The "community" (no-cost) version of C++ desktop development tools and CMake (cmake.org) will build  **NFRL**.

### Visual Studio
To install VS2019 toolset, first download and install the Visual Studio Installer app.  VS2017 will work as well.

1. Navigate to visualstudio.microsoft.com/downloads/ and select Community/Free Download
2. Run the installation wizard.

Next, open the Visual Studio Installer, select:

1. Visual Studio Community 2019
2. Workload *Desktop development with C++*
3. Visual C++ build tools including Windows 10 SDK, CMake, and Redistributable Update in the right-side frame.

Here is an example configuration that successfully builds **NFRL**:

```
{
  "version": "1.0",
  "components": [
    "Microsoft.VisualStudio.Component.CoreEditor",
    "Microsoft.VisualStudio.Workload.CoreEditor",
    "Microsoft.VisualStudio.Component.NuGet",
    "Microsoft.Net.Component.4.6.1.TargetingPack",
    "Microsoft.VisualStudio.Component.Roslyn.Compiler",
    "Microsoft.Net.Component.4.8.SDK",
    "Microsoft.Component.MSBuild",
    "Microsoft.VisualStudio.Component.TextTemplating",
    "Microsoft.VisualStudio.Component.IntelliCode",
    "Microsoft.VisualStudio.Component.VC.CoreIde",
    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
    "Microsoft.VisualStudio.Component.Graphics.Tools",
    "Microsoft.VisualStudio.Component.VC.DiagnosticTools",
    "Microsoft.VisualStudio.Component.Windows10SDK.18362",
    "Microsoft.VisualStudio.Component.Debugger.JustInTime",
    "Microsoft.VisualStudio.Component.VC.Redist.14.Latest",
    "Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core",
    "Microsoft.VisualStudio.ComponentGroup.WebToolsExtensions.CMake",
    "Microsoft.VisualStudio.Component.VC.CMake.Project",
    "Microsoft.VisualStudio.Component.VC.ATL",
    "Microsoft.VisualStudio.Component.VC.TestAdapterForBoostTest",
    "Microsoft.VisualStudio.Component.VC.TestAdapterForGoogleTest",
    "Microsoft.VisualStudio.Component.VC.ATLMFC",
    "Microsoft.VisualStudio.Component.VC.CLI.Support",
    "Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset",
    "Microsoft.VisualStudio.Component.VC.Llvm.Clang",
    "Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Llvm.Clang",
    "Microsoft.VisualStudio.Component.Windows10SDK.17763",
    "Microsoft.VisualStudio.Component.VC.v141.x86.x64",
    "Microsoft.Component.VC.Runtime.UCRTSDK",
    "Microsoft.VisualStudio.Component.VC.140",
    "Microsoft.VisualStudio.Workload.NativeDesktop"
  ]
}
```

### CMake
An installer may be downloaded from Github; the latest version is acceptable.

### OpenCV for Windows
Required packages may be found here: `opencv.org/releases/`.  Download the self-extractor.  For Windows, it is not necessary to build the development and runtime libraries.

Run it to extract the libraries to an appropriate dir, for example, `D:\OpenCV\`.  Rename the root dir generated by the
extraction to include the version number, for example, `D:\OpenCV\opencv-4.5.0`.

Since OpenCV versions 3.4 and 4.5 were tested, they were installed (extracted) *side by side* under the `D:\OpenCV\`
dir:

```
 Directory of D:\OpenCV

02/22/2022  02:00 AM    <DIR>          opencv-3.4.11
02/22/2022  02:00 AM    <DIR>          opencv-4.5.0
```

The `.\build` subdir contains:

- `\include` - headers for compilation
- `\x64\vc15\lib` libraries for linking the build
- `\x64\vc15\bin` dlls for runtime.

## Linux Installations
### OpenCV
Since OpenCV versions 3.4 and 4.5 were tested, they were installed *side by side* under the `/usr/home` dir.  By installing here instead of the default bins and libs dirs, more flexibility is enabled.

*It is not required to install more than one version of OpenCV; nor is it required that OpenCV is installed under the user's home dir.*

Unlike Windows, OpenCV can be built from source.  For any Linux version, go to `https://opencv.org` and click on the Sources link to download the OpenCV source code.  The location of the downloaded archive, the extraction, and the subsequent build, should be a directory, for example, `~/Downloads/OpenCV`, other than the one where the installation occurs.

However, instead of building OpenCV from source, for Ubuntu/Debian, it is possible to install the libopencv-dev package (OpenCV-4.2 as of September 2022) and build NFRL.
#### Extract for build

```
user@:~/Downloads/OpenCV$ unzip opencv-3.4.11.zip
user@:~/Downloads/OpenCV$ unzip opencv-4.5.0.zip
```

#### Installation destination
[Optional: `cmake` will setup install dirs] Create the destination installation folders for the builds (`make install`),
for example:

```
user@:~$ mkdir -p libs/opencv-3.4.11/ libs/opencv-4.5.0/
```

### Ubuntu and Debian
The instructions below are meant only as a guide; your process may vary, however, these steps have been shown
to work for configurations specific to these environments.

#### Update and Upgrade
It is recommended to update the OS to ensure that the latest packages are installed.  Running `sudo apt-get update`
(or equivalent) ensures the list of packages from all repositories and PPAs (Personal Package Archive) is up to date.
If you do not run this command, you could be installing older versions of various packages that could cause dependency
issues.  `apt-get update` does not actually install new versions of software: it updates the package lists for packages
that need upgrading and adds new packages that are recent to the repositories.  Running `sudo apt-get upgrade` installs
the required packages to bring your system up to date.

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Install GNU compilers:

```
$ sudo apt-get install gcc g++
```

#### Required
Quick check the following and install or update if required:

```
$ git --version
git version 2.25.1

$ cmake --version
cmake version 3.16.3
CMake suite maintained and supported by Kitware (kitware.com/cmake).

$ pkg-config --version
0.29.1
```

Run the following and make changes accordingly if requirements are already met:

```
[required] sudo apt-get install pkg-config cmake git libgtk-3-dev libavcodec-dev libavformat-dev libswscale-dev

[optional] sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev
```

Check version Ubuntu (example):

```
$ dpkg -s libgtk-3-dev | grep '^Version'
Version: 3.24.20-0ubuntu1
```

Check version Debian (example):

```
$ dpkg -s libgtk-3-dev | grep '^Version'
Version: 3.24.5-1
```

#### Build OpenCV: `cmake`, `make` and `make install`
In a terminal, change to the root dir of the extracted source, for example, `~/OpenCV/opencv-4.5.0`.  Next, create
a `build` dir and change to it.

Note: to configure OpenCV to build the single, monolithic, "world" lib which includes all OpenCV modules for easy
building (just like with the pre-built versions), add ` -DBUILD_opencv_world=ON ` to the `cmake` command.

Note: don't forget the `..` (up one dir) at the end of the `cmake` command.

```
user@ build]$ cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=~/libs/opencv-4.5.0 -DOPENCV_GENERATE_PKGCONFIG=ON ..  
user@ build]$ make
user@ build]$ sudo make install
```

#### pkg-config
Since generation of pkg-config info was turned ON (per `cmake` command), update/create the `PKG_CONFIG_PATH`
environment variable:

```
$ export PKG_CONFIG_PATH=~/libs/opencv-4.5.0/lib/pkgconfig:~/libs/opencv-3.4.11/lib/pkgconfig:$PKG_CONFIG_PATH
```

Check `pkg-config`:

```
$ pkg-config --cflags opencv4
-I/home/[user]/libs/opencv-4.5.0/include/opencv4
$ pkg-config --libs opencv4
-L/home/[user]/libs/opencv-4.5.0/lib -lopencv_dnn -lopencv_gapi -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core

$ pkg-config --cflags opencv
-I/home/[user]/libs/opencv-3.4.11/include/opencv
$ pkg-config --libs opencv
-L/home/[user]/libs/opencv-3.4.11/lib -lopencv_dnn -lopencv_gapi -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core
```

For convenience, add the `PKG_CONFIG_PATH` export-command to your `~.bashrc` file.

#### Runtime libs PATH

```
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/libs/opencv-4.5.0/lib:~/libs/opencv-3.4.11/lib
```

For convenience, add the `LD_LIBRARY_PATH` export-command to your `~.bashrc` file.

### CentOS 7, 8
#### Required
Quick check the following and install or update if required:

```
$ git --version
git version 2.34.1

$ cmake --version
cmake version 3.17.3
CMake suite maintained and supported by Kitware (kitware.com/cmake).

$ pkg-config --version
0.27.1
```

Install the required tools and libraries.

```
$ sudo yum install epel-release git gcc gcc-c++ cmake \
    gtk2-devel libpng-devel openexr-devel libwebp-devel \
    libjpeg-turbo-devel libtiff-devel gstreamer-plugins-base-devel \
    mesa-libGL mesa-libGL-devel
```

#### Build OpenCV: `cmake`, `make` and `make install`
In a terminal, change to the root dir of the extracted source, for example, `~/OpenCV/opencv-4.5.0`.  Next, create
a `build` dir and change to it.

Note: to configure OpenCV to build the single, monolithic, "world" lib which includes all OpenCV modules for easy
building (just like with the pre-built versions), add ` -DBUILD_opencv_world=ON ` to the `cmake` command.

Note: don't forget the `..` (up one dir) at the end of the `cmake` command.

```
user@ build]$ cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=~/libs/opencv-4.5.0 -DOPENCV_GENERATE_PKGCONFIG=ON ..  
user@ build]$ make
user@ build]$ sudo make install
```

#### pkg-config
Since generation of pkg-config info was turned ON (per `cmake` command), update/create the `PKG_CONFIG_PATH`
environment variable:

```
$ export PKG_CONFIG_PATH=~/libs/opencv-4.5.0/lib64/pkgconfig:~/libs/opencv-3.4.11/lib64/pkgconfig:$PKG_CONFIG_PATH
```

Note: For build of **NFRL**, `pkg-config` is used to point to the OpenCV `include` and `libs` directories.

Check `pkg-config`:

```
$ pkg-config --cflags opencv4
-I/home/[user]/libs/opencv-4.5.0/include/opencv4
$ pkg-config --libs opencv4
-L/home/[user]/libs/opencv-4.5.0/lib64 -lopencv_dnn -lopencv_gapi -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core

$ pkg-config --cflags opencv
-I/home/[user]/libs/opencv-3.4.11/include/opencv
$ pkg-config --libs opencv
-L/home/[user]/libs/opencv-3.4.11/lib64 -lopencv_dnn -lopencv_gapi -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core
```

For convenience, add the `PKG_CONFIG_PATH` export-command to your `~.bashrc` file.

#### Runtime libs PATH

```
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/libs/opencv-4.5.0/lib64:~/libs/opencv-3.4.11/lib64
```

For convenience, add the `LD_LIBRARY_PATH` export-command to your `~.bashrc` file.


# Build NFRL
NFRL's architecture is comprised of a "core" and a "wrapper."

The NFRL-core contains all the essential functionality that NFRL provides.  Its API takes the input-images
as a C++ `std::vector` type.  When built as a static library, NFRL abstracts OpenCV and thus NFRL becomes "portable;"
any using-application that uses NFRL does not require the OpenCV SDK in order to build.

The NFRL-wrapper affords the user to incorporate OpenCV in the using-application.  It "wraps" the NFRL-core.
Most likely, this wrapper-version would be leveraged using OpenCV's functions to read and write images from/to disk.
Its API takes the input images as a `cv::Mat` type.

NFRL build | library name  | image API type
-----------|---------------|-----------
core       | nfrl_core     | C++ `std::vector`
wrapper    | nfrl_opencv   | OpenCV `cv::Mat`

**Table 1 - NFRL build options**

There is NO DIFFERENCE in functionality between the NFRL-core and -wrapper "versions."

## Windows
In the `./NFRL/src/lib/CMakeLists.txt` file inside the `if(WIN32)` block, update the `include` function to point
to OpenCV's `.cmake` file, e.g.:

```
include(D:/OpenCV/opencv-4.5.0/build/x64/vc15/lib/OpenCVConfig.cmake)
```

### NFRL Core
In a Powershell terminal, navigate to the `NFRL/build` directory.  See also `.\doc\build\build_commands.txt` for additional info.
Run as follows:

```
PS .\build> cmake -S .. -DCMAKE_CONFIGURATION_TYPES="Release"
PS .\build> MSBuild nfrl_core.sln
```

The library file is linked into `.\src\lib\Release\nfrl_core.lib`.

In the case that either of the build-commands are not found, update the `PATH` environment variable.  To do this
in Powershell, see Table 2 below.

Here's one way to find `MSBuild.exe`:
```
PS .\NFRL\build> &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe
```
Or, build using full path to `MSBuild`:
```
PS .\NFRL\build> & 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe' nfrl_core.sln
```

Tool    | Cmd to add to PATH  
--------|---------------
cmake   | $Env:PATH += ";C:\Program Files\CMake\bin"    
MSBuild | $Env:PATH += ";C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin"    

**Table 2 - Windows Powershell PATH Env Variable Update**


### NFRL Wrapper
In a Powershell terminal, navigate to the `NFRL/build` directory.  Run as follows:

```
PS .\build> cmake -S .. -DCMAKE_CONFIGURATION_TYPES="Release" -DUSE_OPENCV=1
PS .\build> MSBuild nfrl_opencv.sln
```

The library file is linked into `.\src\lib\Release\nfrl_opencv.lib`.

## Linux
Use these steps for any Linux OS.

In the `./NFRL/src/lib/CMakeLists.txt`, modify the name used by `pkg-config` based on the version of OpenCV to build
against:

- opencv-3.x.x: use `pkg_check_modules(OPENCV REQUIRED opencv)`
- opencv-4.5.x: use `pkg_check_modules(OPENCV REQUIRED opencv4)`

### NFRL Core
In a terminal, navigate to the `NFRL/build` directory.  Run as follows:

```
./build$ cmake ..
./build$ make
```

The library file is linked into `./NFRL/build/src/lib/libnfrl_core.a`.

### NFRL Wrapper
In a terminal, navigate to the `NFRL/build` directory.  Run as follows:

```
./build$ cmake -DUSE_OPENCV=1 ..
./build$ make
```

The library file is linked into `./NFRL/build/src/lib/libnfrl_opencv.a`.

Note that since OpenCV was built from source using the `-DCMAKE_BUILD_TYPE=RELEASE` switch, the release version of
OpenCV is built into the **NFRL** library file.


# Usage of NFRL
## NFRL Core with C++ API
In the using source code, declare a pointer for allocation on the heap.  Catch `NFRL::Miscue` per the
constructor and the registration call:

```
NFRL_ITL::Registrator *r2;

std::vector<uchar> imgMovingData;
std::vector<uchar> imgFixedData;
std::vector<int> controlPointsCoords;
std::vector<std::string> metadataVisualInspection;
try {
  // Instantiate. Exception thrown if either image buffer is empty.
  r2 = new NFRL_ITL::Registrator( imgMovingData, imgFixedData, controlPointsCoords, metadataVisualInspection );
  // Exception thrown if control-point-coords vector COUNT not-equal to 8,
  // control-point overlap, image padding fails, or some other OpenCV exception.
  r2->performRegistration();
}
catch( NFRL::Miscue &e ) {
  std::cout << e.message() << std::endl;
  std::cout << e.what() << std::endl;
}
```

### Get the Registration Results
Registration metadata is available via custom methods and XML.  To retrieve the entire set of registration metadata
in XML format:

```
XmlMetadata xmd;
r2->getXmlMetadata(xmd);    // object r2 already instantiated
for( long unsigned i = 0; i < xmd.size(); i++ ) { std::cout << xmd[i] << std::endl; }
```
Or, access the registration metadata object directly:

```
NFRL_ITL::Registrator::RegistrationMetadata rmd;
r2->getMetadata(rmd);       // object r2 already instantiated
int regImgRows = rmd.registeredImgSize.height;
int regImgCols = rmd.registeredImgSize.width;
std::cout << "REG IMG SIZE: rows: " << regImgRows << " cols: " << regImgCols << std::endl;
```

## NFRL Core with OpenCV API (the Wrapper)
In the using source code, declare a pointer for allocation on the heap.  Catch `NFRL::Miscue` per the
constructor and the registration call:

```
#include <opencv2/highgui/highgui.hpp>

NFRL_ITL::Registrator *r2;

cv::Mat imgMovingData;
cv::Mat imgFixedData;
std::vector<int> controlPointsCoords;
std::vector<std::string> metadataVisualInspection;
try {
  // Instantiate. Exception thrown if either image buffer is empty.
  r2 = new NFRL_ITL::Registrator( imgMovingData, imgFixedData, controlPointsCoords, metadataVisualInspection );
  // Exception thrown if control-point-coords vector COUNT not-equal to 8,
  // control-point overlap, image padding fails, or some other OpenCV exception.
  r2->performRegistration();
}
catch( NFRL::Miscue &e ) {
  std::cout << e.message() << std::endl;
  std::cout << e.what() << std::endl;
}
```

### Get the Registration Results
Registration metadata is available via custom methods and XML.  To retrieve the entire set of registration metadata
in XML format:

```
XmlMetadata xmd;
r2->getXmlMetadata(xmd);    // object r2 already instantiated
for( long unsigned i = 0; i < xmd.size(); i++ ) { std::cout << xmd[i] << std::endl; }
```
Or, access the registration metadata object directly:

```
NFRL::Registrator::RegistrationMetadata rmd;
r2->getMetadata(rmd);       // object r2 already instantiated
int regImgRows = rmd.registeredImgSize.height;
int regImgCols = rmd.registeredImgSize.width;
std::cout << "REG IMG SIZE: rows: " << regImgRows << " cols: " << regImgCols << std::endl;
```

## Display Registration Results
There are six images available for display:

1. Registered, Moving image
2. Registered, Fixed image
3. Overlaid padded, colorized image (for visual inspection)
4. Padded blob region from which ROI coords were calculated; this is a PNG-compressed image
5. Padded, registered Moving image (grayscale)
6. Padded, Fixed image (grayscale)

To display the overlaid image for visual inspection:

```
#include <opencv2/highgui/highgui.hpp>

std::vector<unsigned char> olImg = r2->getColorOverlaidRegisteredImages();     // object r2 already instantiated
try {
  cv::Mat olImage = cv::imdecode( olImg, cv::IMREAD_UNCHANGED );
  cv::imshow( "Overlaid Images", olImage );
  }
catch( const cv::Exception& ex ) {
  std::cout << ex.what() << std::endl;
}
```

If desired to draw the crop region in the registered overlay image using OpenCV:
```
std::vector<int> extractCropROI( std::vector<std::string> strsToSplit )
{
  std::vector<int> output;
  std::string item;
  for( auto x:strsToSplit ) { 
    std::stringstream ss(x);
    while( std::getline(ss, item, ',') )
    {
      output.push_back( std::stoi(item) );
    }
  }
  return output;
}

// rm2 is the registration-metadata instance.
std::vector<std::string> vecCropROI = rm2.overlapROICorners;
cropROIcoords = extractCropROI( vecCropROI );
cv::Rect minRect( cropROIcoords[0], cropROIcoords[1],     // top-left rect
                ( cropROIcoords[2] - cropROIcoords[0] ),  // bot-right rect
                ( cropROIcoords[3] - cropROIcoords[1]) ); // bot-right rect

// Call the appropriate OpenCV display function.
cv::rectangle( img, minRect.tl(), minRect.br(), cv::Scalar(0,0,0), 1 );
cv::imshow( title, img );
```

## Save Registered Images
To save the registered to disk, provide the `PATH`.  If the `PATH` is invalid or the image buffer is empty or otherwise
corrupted, exception is thrown.

```
try {
  r2->saveCroppedRegisteredImageToDisk( std::string PATH );
  r2->saveCroppedFixedImageToDisk( std::string PATH );
}
catch( NFRL::Miscue &e ) {
  std::cout << e.message() << std::endl;
}
```

## Delete
Don't forget to delete the NFRL object.

```
  delete r2;
```

# Doxygen
See README under `./doc` directory.


# Contact
Project Lead: John Libert (john.libert@nist.gov)

Developers NFRL: Bruce Bandini (bruce.bandini@nist.gov)


# Disclaimer
See the NIST disclaimer at https://www.nist.gov/disclaimer
