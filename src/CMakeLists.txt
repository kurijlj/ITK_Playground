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

# Set the source files for the `app` target
add_executable(create_image create_image.cpp)

# Link the `create_image` target with the required libraries
target_link_libraries(create_image PRIVATE
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
add_executable(create_step_wedge create_step_wedge.cpp)

# Link the `create_step_wedge` target with the required libraries
target_link_libraries(create_step_wedge  PRIVATE
  clipp
  ${ITK_LIBRARIES}
)