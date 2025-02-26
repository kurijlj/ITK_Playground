# =============================================================================
# Build source targets
# =============================================================================

# Print message to console that we are building the source targets
message(STATUS "Going through ./src")


# -----------------------------------------------------------------------------
# Target: dummy
# -----------------------------------------------------------------------------
#
# Description: A dummy target that does nothing. This is useful for testing
#              the build tool.
#
# -----------------------------------------------------------------------------

# Show message that we are building the `dummy' target
message(STATUS "Configuring the `dummy' target")

# Add a dummy target
add_custom_target(dummy
    COMMAND ${CMAKE_COMMAND} -E echo "Dummy target"
)


# -----------------------------------------------------------------------------
# Target: create_image
# -----------------------------------------------------------------------------
#
# Description: A simple test of ITK create image and write to image facilities.
#
# -----------------------------------------------------------------------------

# Show message that we are configuring the `create_image' target
message(STATUS "Configuring the `create_image` target")

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Set the source files for the `create_image` target
add_executable(create_image create_image.cxx)

# Link the `create_image` target with the required libraries
target_link_libraries(create_image PRIVATE
  clipp
  ${ITK_LIBRARIES}
)


# -----------------------------------------------------------------------------
# Target: create_image_from_buffer
# -----------------------------------------------------------------------------
#
# Description: Create image from buffer and write it to a file.
#
# -----------------------------------------------------------------------------

# Show message that we are configuring the `create_image_from_buffer' target
message(STATUS "Configuring the `create_image_from_buffer` target")

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Set the source files for the `create_image_from_buffer` target
add_executable(create_image_from_buffer create_image_from_buffer.cxx)

# Link the `create_image_from_buffer` target with the required libraries
target_link_libraries(create_image_from_buffer PRIVATE
  clipp
  ${ITK_LIBRARIES}
)


# -----------------------------------------------------------------------------
# Target: create_step_wedge
# -----------------------------------------------------------------------------
#
# Description: Create computational optical density step wedge image.
#
# -----------------------------------------------------------------------------

# Show message that we are configuring the `create_step_wedge' target
message(STATUS "Configuring the `create_step_wedge` target")

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Set the source files for the `create_step_wedge` target
add_executable(create_step_wedge create_step_wedge.cxx)

# Link the `create_step_wedge` target with the required libraries
target_link_libraries(create_step_wedge  PRIVATE
  clipp
  ${ITK_LIBRARIES}
)


# -----------------------------------------------------------------------------
# Target: image_affine_transform
# -----------------------------------------------------------------------------
#
# Description: Rotate and translate an image using ITK.
#
# -----------------------------------------------------------------------------

# Show message that we are configuring the `image_affine_transform' target
message(STATUS "Configuring the `image_affine_transform` target")

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Set the source files for the `image_affine_transform` target
add_executable(image_affine_transform image_affine_transform.cxx)

# Link the `image_affine_transform` target with the required libraries
target_link_libraries(image_affine_transform  PRIVATE
  clipp
  ${ITK_LIBRARIES}
)


# -----------------------------------------------------------------------------
# Target: split_channels
# -----------------------------------------------------------------------------
#
# Description: Split color channels of an image.
#
# -----------------------------------------------------------------------------

# Show message that we are configuring the `split_channels' target
message(STATUS "Configuring the `split_channels` target")

# Find required libraries and packages
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(Git REQUIRED)
if (NOT Git_FOUND)
  message(FATAL_ERROR "Git not found!")
endif()

# Set the source files for the `split_channels` target
add_executable(split_channels split_channels.cxx )

# Link the `split_channels` target with the required libraries
target_link_libraries(split_channels  PRIVATE
  clipp
  ${ITK_LIBRARIES}
)


# -----------------------------------------------------------------------------
# Target: rgb_to_luminance
# -----------------------------------------------------------------------------
#
# Description: Convert RGB image to luminance image.
#
# -----------------------------------------------------------------------------

# Show message that we are configuring the `rgb_to_luminance' target
message(STATUS "Configuring the `rgb_to_luminance` target")

# Find required libraries and packages
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(Git REQUIRED)
if (NOT Git_FOUND)
  message(FATAL_ERROR "Git not found!")
endif()

# Set the source files for the `split_channels` target
add_executable(rgb_to_luminance rgb_to_luminance.cxx )

# Link the `split_channels` target with the required libraries
target_link_libraries(rgb_to_luminance  PRIVATE
  clipp
  ${ITK_LIBRARIES}
)
