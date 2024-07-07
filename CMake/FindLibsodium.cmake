# =============================================================================
# FindLibsodium - Find the cryptographic library Libsodium
# =============================================================================
#
# Description:
#   This module finds the cryptographic library Libsodium and provides
#   information about its location and version.
#
# Usage:
#   To find Libsodium, simply use
#
#     find_package(Libsodium)
#
#   and then use the following variables:
#
#     Libsodium_FOUND        - System has Libsodium
#     Libsodium_INCLUDE_DIR  - The Libsodium include directory
#     Libsodium_LIBRARIES    - The libraries to link against
#     Libsodium_VERSION      - The version of Libsodium found
#
#   If you want to use a specific version of Libsodium, you can set the
#   following variables:
#
#     Libsodium_ROOT         - The root directory of Libsodium
#     Libsodium_FIND_VERSION - The version of Libsodium to find. This can be
#                              set through find_package() or by setting the
#                              variable directly.
#
#   If you want to link against the Libsodium library as dynamically or
#   statically, please set the BUILD_SHARED_LIBS variable before calling
#   find_package(Libsodium).
#
#   If liniking as static library, do not forget to put the:
#       #define SODIUM_STATIC 1
#   before including the sodium.h header file.
#
# Copyright (c) 2024, Ljubomir Kurij <ljubomir_kurij@protonmail.com>
#
# =============================================================================

# =============================================================================
# TODO:
# =============================================================================
#
# * Add support for more versions of Libsodium
# * Test with other compilers than MSVC
# * Test with other operating systems than Windows
# * Test how module behaves when invoked multiple times
#
# =============================================================================

# -----------------------------------------------------------------------------
# Set the list of supported versions
# -----------------------------------------------------------------------------
# This is due to the fact that the directory structure of the library
# changes between versions and we need to know where to look for the
# libraries. This is only necessary for MSVC, as the library is usually
# installed in the system on Unix-like systems.
# -----------------------------------------------------------------------------
if (MSVC)
    set (_supported_versions
        "1.4.2"
        "1.4.3"
        )
else ()
    set (_supported_versions
        "1.0.18"
        "1.0.19"
        )
endif (MSVC)

# -----------------------------------------------------------------------------
# If user has not set Libsodium_ROOT, then use the default path
# -----------------------------------------------------------------------------
# This is useful for users who have put the library in a non-standard
# location, e.g. in the extern directory of their project.
# -----------------------------------------------------------------------------
if (NOT Libsodium_ROOT)
    set (Libsodium_ROOT "${PROJECT_SOURCE_DIR}/extern/libsodium")
endif (NOT Libsodium_ROOT)

find_path (
    Libsodium_INCLUDE_DIRS
    NAMES
        sodium.h
    PATHS
        /usr/include
        /usr/local/include
        "${Libsodium_ROOT}/include"
    DOC "The Libsodium include directory"
    )
mark_as_advanced (Libsodium_INCLUDE_DIRS)  # Hide this variable from the user

# Check if user has requested a specific version
if (Libsodium_FIND_VERSION)
    list (FIND _supported_versions "${Libsodium_FIND_VERSION}" _index)
    if (_index EQUAL -1)
        message (FATAL_ERROR
            "Unsupported Libsodium version: ${Libsodium_FIND_VERSION}"
            )
    endif ()
    set (_version "${Libsodium_FIND_VERSION}")
else ()
    list (GET _supported_versions 0 _version)  # Use the latest version
endif (Libsodium_FIND_VERSION)

# If we are using MSVC, point to the correct directory
if (MSVC)
    # Path to the Libsodium libraries for MSVC inside the project directory
    # is generated from the architecture, build configuration, version of
    # the library and whether the library is linked statically or dynamically
    set (_libsodium_lib_dir "${Libsodium_ROOT}")
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (_libsodium_lib_dir "${_libsodium_lib_dir}/x64")
    else ()
        set (_libsodium_lib_dir "${_libsodium_lib_dir}/Win32")
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set (_libsodium_lib_dir "${_libsodium_lib_dir}/Debug")
    else ()
        set (_libsodium_lib_dir "${_libsodium_lib_dir}/Release")
    endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

    # Generate the version path from the _version variable
    string (REPLACE "." "" _version_path "v${_version}")
    set (_libsodium_lib_dir "${_libsodium_lib_dir}/${_version_path}")

    # Check if the library is linked statically or dynamically
    if (BUILD_SHARED_LIBS)
        set (_libsodium_lib_dir "${_libsodium_lib_dir}/dynamic")
    else ()
        set (_libsodium_lib_dir "${_libsodium_lib_dir}/static")
    endif (BUILD_SHARED_LIBS)

    find_library (
        Libsodium_LIBRARY
        NAMES
            sodium
            libsodium
        PATHS
            "${_libsodium_lib_dir}"
        REQUIRED
        DOC "The libraries to link against"
        )

    # Find the shared library for MSVC
    find_path (
        Libsodium_SHARED_LIBRARY
        NAMES
            libsodium.dll
        PATHS
            /usr/lib
            /usr/local/lib
            "${_libsodium_lib_dir}"
        DOC "The Libsodium *.dll directory"
        )

else ()
    find_library (
        Libsodium_LIBRARY
        NAMES
            sodium
            libsodium
        PATHS
            /usr/lib
            /usr/local/lib
            "${Libsodium_ROOT}/lib"
        REQUIRED
        DOC "The libraries to link against"
        )
endif (MSVC)
mark_as_advanced (Libsodium_LIBRARY)  # Hide this variable from the user

# Handle the QUIETLY and REQUIRED arguments and set Libsodium_FOUND to TRUE if
# all listed variables are TRUE
set (Libsodium_VERSION "${_version}")
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (
    Libsodium
    DEFAULT_MSG
    Libsodium_LIBRARY
    Libsodium_INCLUDE_DIRS
    Libsodium_VERSION
    )

# If Libsodium was found, then create an imported target
if (Libsodium_FOUND)
    set (Libsodium_FOUND TRUE)
    set (Libsodium_INCLUDE_DIR "${Libsodium_INCLUDE_DIRS}")
    set (Libsodium_LIBRARIES "${Libsodium_LIBRARY}")
    if (NOT TARGET Libsodium)
        # Determine the link type
        if (BUILD_SHARED_LIBS)
            set (_link_type "SHARED")
        else ()
            set (_link_type "STATIC")
        endif (BUILD_SHARED_LIBS)

        # Create the imported target
        add_library (Libsodium ${_link_type} IMPORTED)
        set_target_properties (
            Libsodium
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Libsodium_INCLUDE_DIR}"
                IMPORTED_LOCATION "${Libsodium_LIBRARIES}"
                IMPORTED_IMPLIB "${Libsodium_LIBRARIES}"
            )
        
        # Add the imported target to the global list of targets
        # (do we really need this?)
        set_property (GLOBAL APPEND PROPERTY Libsodium::Libsodium Libsodium)

        # Hide the imported target from the user
        set_property (TARGET Libsodium PROPERTY IMPORTED_GLOBAL TRUE)
    endif (NOT TARGET Libsodium)
else ()
    set (Libsodium_FOUND FALSE)
endif (Libsodium_FOUND)