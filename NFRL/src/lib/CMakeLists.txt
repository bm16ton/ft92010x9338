
if(USE_OPENCV)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENCV")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
add_library( ${PROJECT_NAME}
  nfrl_itl.cpp
  nfrl_lib.cpp
  corresponding_points_pair.cpp
  corresponding_points_pairs.cpp
  opencv_procs.cpp
  overlap_registered_images.cpp
  points_on_image.cpp
  points_on_images.cpp
)
else()
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
add_library( ${PROJECT_NAME}
  nfrl_lib.cpp
  corresponding_points_pair.cpp
  corresponding_points_pairs.cpp
  opencv_procs.cpp
  overlap_registered_images.cpp
  points_on_image.cpp
  points_on_images.cpp
)

endif()


if(WIN32)
  # To access OpenCV, use either these two lines below:
  include(D:/OpenCV/opencv-4.5.0/build/x64/vc15/lib/OpenCVConfig.cmake)
  target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

  # Or to access OpenCV, use these two lines below:
  # set(OpenCV_DIR "D:/OpenCV/opencv-4.5.0/build/x64/vc15/lib/")
  # find_package(OpenCV REQUIRED)

  message(STATUS "OpenCV_INCLUDE_DIRS found as: '${OpenCV_INCLUDE_DIRS}'")
  message(STATUS "OpenCV_LIBS found as: '${OpenCV_LIBS}'")

  # line below ALWAYS required
  target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})

else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(OPENCV REQUIRED opencv4)
	target_link_libraries(${PROJECT_NAME} ${OPENCV_LINK_LIBRARIES})
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENCV_INCLUDE_DIRS})
	target_compile_options(${PROJECT_NAME} PUBLIC ${OPENCV_CFLAGS_OTHER})

  message(STATUS "OPENCV_INCLUDE_DIRS found as: '${OPENCV_INCLUDE_DIRS}'")
	message(STATUS "OPENCV_LIBRARIES found as: '${OPENCV_LINK_LIBRARIES}'")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
