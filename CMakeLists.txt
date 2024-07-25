# ============================================================================
# A CMake build script for the C++ CLI application using the clipp.h library.
# ============================================================================

# Define the minimum required version of CMake
cmake_minimum_required (VERSION 3.12...3.40)

# Define the project name and version
project (
  cli_framework
  VERSION 0.1
  DESCRIPTION "Self-paced exploration of the ITK (Insight Toolkit) library."
  HOMEPAGE_URL ""
  LANGUAGES C CXX
  )

# Set the C++ standard to C++17 and make it required
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type to Release if not specified
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif ()

# Set to build with shared libraries by default
option (BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Determine whether the libraries are built as shared or static
if (BUILD_SHARED_LIBS)
  set (LIB_TYPE SHARED)
else ()
  set (LIB_TYPE STATIC)
endif (BUILD_SHARED_LIBS)

# Set the output directory for the executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the clipp.h library
add_library (clipp INTERFACE)
target_sources (clipp INTERFACE "${PROJECT_SOURCE_DIR}/extern/clipp/clipp.hpp")
target_include_directories (
  clipp
  INTERFACE
  "${PROJECT_SOURCE_DIR}/extern/clipp"
  )

# Enable FetchContent module
include(FetchContent)

# Add the ITK library
FetchContent_Declare(
  ITK
  URL https://github.com/InsightSoftwareConsortium/ITK/releases/download/v5.3.0/InsightToolkit-5.3.0.zip 
  )
FetchContent_MakeAvailable(ITK)
set(ITK_DIR ${ITK_BINARY_DIR} CACHE STRING "Make the ITK_DIR available to other modules" FORCE)

# Patch the ITK library with the custom code
configure_file (
  "${PROJECT_SOURCE_DIR}/extern/itk/itkTIFFImageIO.h.in"
  "${ITK_SOURCE_DIR}/Modules/IO/TIFF/include/itkTIFFImageIO.h"
  COPYONLY
  )
configure_file (
  "${PROJECT_SOURCE_DIR}/extern/itk/itkTIFFImageIO.cxx.in"
  "${ITK_SOURCE_DIR}/Modules/IO/TIFF/src/itkTIFFImageIO.cxx"
  COPYONLY
  )

# Add the test TIFF images
FetchContent_Declare(
  test_images
  URL https://download.osgeo.org/libtiff/pics-3.8.0.tar.gz
  )
FetchContent_MakeAvailable(test_images)

# Add the source files directory
add_subdirectory ("${PROJECT_SOURCE_DIR}/src")

# Check if the tests are enabled
if (NOT DEFINED BUILD_TESTS)
    option (BUILD_TESTS "Build the tests" OFF)  # Default to OFF
endif ()

# Add the test files directory if the tests are enabled
if (BUILD_TESTS)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    # For Windows: Prevent overriding the parent project's
    # compiler/linker settings
    set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    add_subdirectory ("${PROJECT_SOURCE_DIR}/tests")

endif ()
