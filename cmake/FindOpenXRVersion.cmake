# Determine the major/minor/patch version from the vulkan header
set(OPENXR_VERSION_MAJOR "")
set(OPENXR_VERSION_MINOR "")
set(OPENXR_VERSION_PATCH "")

# Temporarily store the sysroot and prefix path items and then modify those
# variables to set CMake so that we're only searching local tree
set(OLD_CMAKE_SYSROOT ${CMAKE_SYSROOT})
set(OLD_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
set(OLD_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
set(OLD_CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})
set(CMAKE_SYSROOT "")
set(CMAKE_PREFIX_PATH "")
set(CMAKE_FIND_ROOT_PATH "")
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)

# First, determine which header we need to grab the version information from.
find_file (OPENXR_HEADER
            openxr.h
            HINTS
                ${GFXRECON_SOURCE_DIR}/external/OpenXR-SDK/include/openxr
                ${CMAKE_SOURCE_DIR}/external/OpenXR-SDK/include/openxr
                ${CMAKE_SOURCE_DIR}/../external/OpenXR-SDK/include/openxr
                ${CMAKE_SOURCE_DIR}/../../external/OpenXR-SDK/include/openxr
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/OpenXR-SDK/include/openxr
                ${CMAKE_CURRENT_SOURCE_DIR}/../../external/OpenXR-SDK/include/openxr
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/OpenXR-SDK/include/openxr
          )

MESSAGE(STATUS "OpenXR Header = ${OPENXR_HEADER}")

if (EXISTS ${OPENXR_HEADER})
    set(OpenXRHeaders_main_header ${OPENXR_HEADER})
endif()

# Find all lines in the header file that contain any version we may be interested in
#  NOTE: They start with #define and then have other keywords
file(STRINGS
        ${OpenXRHeaders_main_header}
        OpenXRHeaders_lines
        REGEX "^#[ \t]*define[ \t]*XR_CURRENT_API_VERSION")

foreach(OpenXRHeaders_line ${OpenXRHeaders_lines})
    # First, handle the case where we have a major/minor version
    #   Format is:
    #        #define XR_CURRENT_API_VERSION XR_MAKE_VERSION(X, Y, XR_HEADER_VERSION)
    #   We grab the major version (X) and minor version (Y) out of the parentheses
    string(REGEX MATCH "define[ \t]+XR_CURRENT_API_VERSION[ \t]+XR_MAKE_VERSION\\(.*\\)" OpenXRHeaders_make_version ${OpenXRHeaders_line})
    string(REGEX MATCHALL "[0-9]+" OpenXRHeaders_Version "${OpenXRHeaders_make_version}")
    if (OpenXRHeaders_Version)
        list (GET OpenXRHeaders_Version 0 XR_VERSION_MAJOR)
        list (GET OpenXRHeaders_Version 1 XR_VERSION_MINOR)
        list (GET OpenXRHeaders_Version 2 XR_VERSION_PATCH)
    endif()
endforeach()

if (XR_VERSION_MAJOR STREQUAL "")
        message(FATAL_ERROR "Failed to find major version in " ${OpenXRHeaders_main_header})
endif()

if (XR_VERSION_MINOR STREQUAL "")
        message(FATAL_ERROR "Failed to find minor version in " ${OpenXRHeaders_main_header})
endif()

if (XR_VERSION_PATCH STREQUAL "")
        message(FATAL_ERROR "Failed to find patch version in " ${OpenXRHeaders_main_header})
endif()

MESSAGE(STATUS
        "Detected OpenXR Version ${XR_VERSION_MAJOR}."
        "${XR_VERSION_MINOR}."
        "${XR_VERSION_PATCH}")

# Restore the sysroot and prefix path CMake variables
set(CMAKE_SYSROOT ${OLD_CMAKE_SYSROOT})
set(CMAKE_PREFIX_PATH ${OLD_CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH ${OLD_CMAKE_FIND_ROOT_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${OLD_CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})