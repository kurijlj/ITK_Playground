# =============================================================================
# Build source targets
# =============================================================================

# Print message to console that we are building the source targets
message(STATUS "Going through ./src")

# -----------------------------------------------------------------------------
# app
# -----------------------------------------------------------------------------

# Show message that we are building the app target
message(STATUS "Configuring the `app` target")

# Set the source files for the `app` target
add_executable(app app.cpp)

# Link the `app` target with the required libraries
target_link_libraries(app PRIVATE
    clipp
    sqlite3
    Libsodium
    csv
)

# Include the required directories for the `app` target
target_include_directories(app PRIVATE
    ${CSV_INCLUDE_DIR}
)

# Post-build actions for the `app` target
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/sample_data
        ${CMAKE_CURRENT_BINARY_DIR}/sample_data
)

if (BUILD_SHARED_LIBS AND WIN32)
    # Copy the shared libraries to the output directory
    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${Libsodium_SHARED_LIBRARY}/libsodium.dll"
            "$<TARGET_FILE_DIR:app>"
    )
endif ()


# -----------------------------------------------------------------------------
# stat_demo
# -----------------------------------------------------------------------------

# Show message that we are building the `stat_demo` target
message(STATUS "Configuring the `stat_demo` target")

# Set the source files for the `stat_demo` target
add_executable(stat_demo stat_demo.cpp)

# Link the `stat_demo` target with the required libraries
target_link_libraries(stat_demo PRIVATE
    clipp
)