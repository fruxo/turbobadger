#
# CMake file for hastybadger
#
project (TurboBadger CXX)
cmake_minimum_required (VERSION 3.4.3)
include (CheckCXXCompilerFlag)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set (VERSION_MAJOR "0")
set (VERSION_MINOR "1")
set (VERSION_PATCH "0")
set (VERSION_BUILD "-beta3") # ie "-beta1", or empty string if release: ""
set (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

#
# This approach to labeling beta versions is in line with Apple's
# inflexible versioning scheme for pre-release beta testing versions,
# for both iOS and macOS.  Apple allows, ie, FlightTest to release
# versions like "1.0.0b1" (b2,b2,...) as a pre-release beta for
# "1.0.0".  There is no other possible beta numbering scheme in Apple
# world, dont even bother trying! :D This labelling with "-beta2" or
# "" for releases also plays well with the other package generators.
#
if (VERSION_BUILD)
  string (REGEX MATCH "[0-9]+" VERSION_BUILD_NUM ${VERSION_BUILD})
else ()
  set (VERSION_BUILD_NUM "")
endif ()
#################################################################################

# Check if we're being built as an add_subdirectory() from another project
get_directory_property (TB_SUBDIRECTORY PARENT_DIRECTORY)

# try to auto-detect TB_SYSTEM etc...
if (UNIX)
  set (TB_SYSTEM_DEFAULT LINUX)
  set (TB_RENDERER_DEFAULT GL3)
  set (TB_BACKEND_DEFAULT GLFW)
endif (UNIX)
if (WIN32)
  set (TB_SYSTEM_DEFAULT WINDOWS)
  set (TB_RENDERER_DEFAULT GL3)
  set (TB_BACKEND_DEFAULT GLFW)
endif (WIN32)
if (APPLE)
  if (IOS)
    set (TB_SYSTEM_DEFAULT IOS)
    set (TB_RENDERER_DEFAULT GLES_2)
    set (TB_BACKEND_DEFAULT SDL2)
    #set (TB_BUILD_SDL2 ON CACHE STRING "Build SDL2")
  else (IOS)
    set (TB_SYSTEM_DEFAULT MACOSX)
    set (TB_RENDERER_DEFAULT GL3)
    set (TB_BACKEND_DEFAULT SDL2)
  endif (IOS)
endif (APPLE)
if (ANDROID)
  set (TB_SYSTEM_DEFAULT ANDROID)
  set (TB_RENDERER_DEFAULT GL)
  set (TB_BACKEND_DEFAULT ANDROID)
endif (ANDROID)

# Special handling for building DEMO app
set (TB_BUILD_DEMO NO CACHE STRING "Build Demo application: NO, GLFW, SDL2, or ANDROID")
set_property (CACHE TB_BUILD_DEMO PROPERTY STRINGS NO GLFW SDL2 ANDROID)

if (${TB_BUILD_DEMO} STREQUAL GLFW)
  set (TB_CLIPBOARD_DEFAULT GLFW)
  set (TB_IMAGE_LOADER_STB ON)
  set (TB_BACKEND_DEFAULT GLFW)
elseif (${TB_BUILD_DEMO} STREQUAL SDL2)
  set (TB_CLIPBOARD_DEFAULT SDL2)
  set (TB_IMAGE_LOADER_STB ON)
  set (TB_BACKEND_DEFAULT SDL2)
elseif (${TB_BUILD_DEMO} STREQUAL ANDROID)
  set (TB_CLIPBOARD_DEFAULT DUMMY)
  set (TB_SYSTEM ANDROID)
  set (TB_IMAGE_LOADER_STB ON)
  set (TB_BACKEND_DEFAULT ANDROID)
endif (${TB_BUILD_DEMO} STREQUAL GLFW)

# The target system
set (TB_SYSTEM "${TB_SYSTEM_DEFAULT}" CACHE STRING "System type")
set_property (CACHE TB_SYSTEM PROPERTY STRINGS LINUX MACOSX WINDOWS ANDROID IOS)

# The backened to use
set (TB_BACKEND "${TB_BACKEND_DEFAULT}" CACHE STRING "Backend")
set_property (CACHE TB_BACKEND PROPERTY STRINGS GLFW SDL2 ANDROID)

if (TB_BACKEND STREQUAL GLFW)
elseif (TB_BACKEND STREQUAL SDL2)
elseif (${TB_BACKEND} STREQUAL ANDROID)
  set (TB_CLIPBOARD_DEFAULT DUMMY)
endif (TB_BACKEND STREQUAL GLFW)

# Should any of the backend libraries be built?
option (TB_BUILD_GLFW "Download and build the GLFW library." OFF)
option (TB_BUILD_SDL2 "Download and build the SDL2 library." OFF)
option (TB_BUILD_FREETYPE "Download and build the freetype library." OFF)

# Which types to generate for for Xcode and VS, default to Release
if (XCODE OR MSVS AND NOT CMAKE_CONFIGURATION_TYPES)
  set (CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo")
endif (XCODE OR MSVS AND NOT CMAKE_CONFIGURATION_TYPES)

# Default CMAKE_BUILD_TYPE, if not Xcode or VS
set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Set cmake build type")
set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set (CMAKE_BUILD_TYPE "Debug")
endif (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

# Configure graphics renderer
option (TB_RENDERER_BATCHER "Enable to get TBRendererBatcher" ON)
if (TB_RENDERER_BATCHER)
  set (TB_RENDERER_BATCHER_CONFIG "#define TB_RENDERER_BATCHER")
endif ()

# Configure renderer
set (TB_RENDERER "${TB_RENDERER_DEFAULT}" CACHE STRING "Which Renderer")
set_property (CACHE TB_RENDERER PROPERTY STRINGS STUB GL GLES_1 GLES_2 GL3 SDL2)

# Enable images
option (TB_IMAGE "Enable support for TBImage, TBImageManager, TBImageWidget." ON)
if (TB_IMAGE)
  set (TB_IMAGE_CONFIG "#define TB_IMAGE")
  set (TB_IMAGE_LOADER_DEFAULT STB)
endif ()
set (TB_IMAGE_LOADER "${TB_IMAGE_LOADER_DEFAULT}" CACHE STRING "Which image loader (def: STB)")

# Runtime/subsystem configurations - check if a Demo is selected

# System defaults
set (TB_FONT_RENDERER_DEFAULT TBBF)
if (${TB_SYSTEM} STREQUAL ANDROID)
  set (TB_CLIPBOARD_DEFAULT DUMMY)
  set (TB_FILE_DEFAULT ANDROID)
endif ()
if (${TB_SYSTEM} STREQUAL LINUX)
  set (TB_FILE_DEFAULT POSIX)
endif ()
if (${TB_SYSTEM} STREQUAL MACOSX)
  set (TB_FILE_DEFAULT POSIX)
endif ()
if (${TB_SYSTEM} STREQUAL WINDOWS)
  set (TB_CLIPBOARD_DEFAULT WINDOWS)
  set (TB_FILE_DEFAULT POSIX)
endif ()

# Clipboard
set (TB_CLIPBOARD ${TB_CLIPBOARD_DEFAULT} CACHE STRING "Clipboard type: DUMMY GLFW SDL2 WINDOWS")
set_property (CACHE TB_CLIPBOARD PROPERTY STRINGS DUMMY GLFW SDL2 WINDOWS)

# Filesystem
set (TB_FILE ${TB_FILE_DEFAULT} CACHE STRING "Filesystem type: OFF WINDOWS POSIX SDL2 ANDROID")
set_property (CACHE TB_FILE PROPERTY STRINGS "OFF" "WINDOWS" "POSIX" "SDL2" "ANDROID")
set (TB_FILE_POSIX_PREFIX ""  CACHE STRING "Prefix string added to beginning of resource paths for TB_FILE_POSIX")
if (TB_FILE_POSIX_PREFIX AND TB_FILE STREQUAL POSIX)
  set (TB_FILE_POSIX_PREFIX_CONFIG "#define TB_FILE_POSIX_PREFIX ${TB_FILE_POSIX_PREFIX}")
endif ()

# Library postfix
option (TB_DEBUG_POSTFIX "Add _d postfix for debug builds." OFF)
if (TB_DEBUG_POSTFIX)
  set (CMAKE_DEBUG_POSTFIX _d)
endif ()

# Configure Font Renderer
set (TB_FONT_RENDERER ${TB_FONT_RENDERER_DEFAULT} CACHE STRING "Select font renderer: TBBF, FREETYPE, STB")
set_property (CACHE TB_FONT_RENDERER PROPERTY STRINGS "TBBF" "FREETYPE" "STB")

# Generate the config header file
configure_file (tb_config.h.in src/tb/tb_config.h)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  add_compile_options (-g -fno-exceptions -fno-rtti -Wall -Wextra -Wno-unused-parameter)
endif ()

if (MSVC)
  add_definitions (-D_CRT_SECURE_NO_WARNINGS)
  add_definitions (-D_CRT_NONSTDC_NO_DEPRECATE)
  # Remove exception handling
  string (REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  # Remove rtti
  string (REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
endif (MSVC)

#
# Look for libraries
if (TB_BACKEND STREQUAL GLFW)
  if (NOT TB_BUILD_GLFW)
    # Add glfw and gl libs on MacOS
    if (APPLE)
      find_library (GLFW_LIBRARIES glfw)
      set (EXTRA_LIBS ${EXTRA_LIBS} ${GLFW_LIBRARIES})
      find_path (GLFW_INCLUDE_DIRS GLFW/glfw3.h)
    else (APPLE)
      find_library (GLFW_LIBRARIES glfw)
    endif (APPLE)
  endif (NOT TB_BUILD_GLFW)
endif (TB_BACKEND STREQUAL GLFW)

if (TB_SYSTEM STREQUAL SDL2)
  if (NOT TB_BUILD_SDL2 AND NOT EMSCRIPTEN)
    include (FindPkgConfig)
    pkg_search_module (SDL2 REQUIRED sdl2)
    link_directories (${SDL2_LIBRARY_DIRS})
    if (APPLE)
      find_library (SDL2_LIBRARIES SDL2)
      set (EXTRA_LIBS ${EXTRA_LIBS} ${SDL2_LIBRARIES})
    else (APPLE)
      find_library (SDL2_LIBRARIES SDL2)
    endif (APPLE)
  endif (NOT TB_BUILD_SDL2 AND NOT EMSCRIPTEN)
endif (TB_SYSTEM STREQUAL SDL2)

add_definitions (-DTB_RUNTIME_DEBUG_INFO=1)

if (EMSCRIPTEN)
  #add_compile_options ("-O2")
  #add_compile_options ("-s;FULL_ES2=1")
  #add_compile_options ("-s EVAL_CTORS=1")
  #add_compile_options ("-s BINARYEN_TRAP_MODE=clamp")
  #add_compile_options ("-s NO_EXIT_RUNTIME=1")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 ")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s FULL_ES2=1")
  # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Oz") # optional
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EVAL_CTORS=1")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s BINARYEN_TRAP_MODE=clamp")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
  set (TB_BUILD_FREETYPE OFF CACHE INTERNAL "auto")
  set (TB_BUILD_SDL2 OFF CACHE INTERNAL "auto")
  set (TB_BUILD_GLFW OFF CACHE INTERNAL "auto")
endif (EMSCRIPTEN)

#################################################################################
#
# Add thirdparty dependencies
#
file (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/thirdparty")

# Generator name
if (NOT "${CMAKE_EXTRA_GENERATOR}" STREQUAL "")
  set (GENERATOR_STRING "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else ()
  set (GENERATOR_STRING "${CMAKE_GENERATOR}")
endif ()

# Configure the thirdparty build directory
execute_process (
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/thirdparty"
  COMMAND ${CMAKE_COMMAND} -G${GENERATOR_STRING} -DANDROID=${ANDROID} -DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -DTB_BUILD_GLFW=${TB_BUILD_GLFW} -DTB_BUILD_SDL2=${TB_BUILD_SDL2} -DTB_BUILD_FREETYPE=${TB_BUILD_FREETYPE} ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
  RESULT_VARIABLE THIRDPARTY_CONFIG_RESULT
  OUTPUT_VARIABLE THIRDPARTY_OUTPUT
  ERROR_VARIABLE THIRDPARTY_OUTPUT
)
if (THIRDPARTY_CONFIG_RESULT)
  message (FATAL_ERROR "Configuring dependencies failed: ${THIRDPARTY_CONFIG_RESULT}, ${THIRDPARTY_OUTPUT}")
endif ()

# Build the thirdparty directory
execute_process (
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/thirdparty"
  COMMAND "${CMAKE_COMMAND}" --build .
  RESULT_VARIABLE THIRDPARTY_BUILD_RESULT
  )
if (THIRDPARTY_BUILD_RESULT)
  message (FATAL_ERROR "Building dependencies failed: ${THIRDPARTY_BUILD_RESULT}")
endif ()

# Include the now-built thirdparty dependencies
add_subdirectory (thirdparty EXCLUDE_FROM_ALL)

#################################################################################
#
# Macros to add resource files
#
define_property (SOURCE PROPERTY RES_ROOT
  BRIEF_DOCS "Chroot base directory, gets chopped"
  FULL_DOCS "The base directory which should be truncated")

macro (add_resources RES_LIST_NAME RES_ROOT_PATH)
  #
  # add file resources to the variable RES_LIST_NAME
  # also tags the source files with the RES_ROOT_PATH using the custom property RES_ROOT
  #
  # example: add_resources(RES_FILES "${CMAKE_SOURCE_DIR}/resources" "*.txt" "*.png" "*.svg")
  #
  foreach (globby ${ARGN})
    file (GLOB_RECURSE RES_FILES_ "${RES_ROOT_PATH}/${globby}")
    if (RES_FILES_)
      set_source_files_properties (${RES_FILES_} PROPERTIES RES_ROOT "${RES_ROOT_PATH}")
      list (APPEND ${RES_LIST_NAME} ${RES_FILES_})
      #message ("f-${RES_LIST_NAME}: '${RES_ROOT_PATH}/${globby}' -> ${RES_FILES_} : ${RES_ROOT}")
    endif ()
  endforeach ()
endmacro ()

macro (fixup_resources RES_LIST_NAME)
  #
  # example: fixup_resources(RES_FILES)
  #
  foreach (RES_FILE IN LISTS ${RES_LIST_NAME})

    # Get the original root-absolute pah
    get_property (RES_ROOT_PATH SOURCE ${RES_FILE} PROPERTY RES_ROOT)

    # Get the relative path from the data-folder to the particular file
    file (RELATIVE_PATH RES_PATH ${RES_ROOT_PATH} ${RES_FILE})
    get_filename_component (RES_DIR ${RES_PATH} DIRECTORY)
    
    # Set it's location inside the app package (under Resources)
    set_property (SOURCE ${RES_FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${RES_DIR}")

    #message ("${RES_FILE} -> Resources/${RES_PATH}")
  endforeach ()
endmacro ()

#################################################################################
#
# Add TurboBadgerLib project
#
if (TB_BUILD_DEMO)
  add_subdirectory (src/tb EXCLUDE_FROM_ALL)
else ()
  add_subdirectory (src/tb)
endif (TB_BUILD_DEMO)

# Add Demo project
if (TB_BUILD_DEMO STREQUAL GLFW OR TB_BUILD_DEMO STREQUAL SDL2)
  add_subdirectory (Demo)
endif (TB_BUILD_DEMO STREQUAL GLFW OR TB_BUILD_DEMO STREQUAL SDL2)

# Android demo
if (${TB_BUILD_DEMO} STREQUAL ANDROID)
  target_compile_definitions (TurboBadgerLib PUBLIC)
  add_library (TurboBadger SHARED
    ./DemoAndroid2/app/src/main/jni/jni_glue.cpp
    ./DemoAndroid2/app/src/main/jni/App.cpp
    )
  target_link_libraries (TurboBadger TurboBadgerLib)
  set_property (TARGET TurboBadger APPEND PROPERTY ANDROID_ASSETS_DIRECTORIES
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/language"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/default_skin")
endif (${TB_BUILD_DEMO} STREQUAL ANDROID)

#################################################################################
# Generate Doxygen documentation
#
if (NOT TB_SUBDIRECTORY)
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile)
  add_custom_target (docs
    COMMAND doxygen ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
    # COMMAND open doc/html/index.html
    )
endif (NOT TB_SUBDIRECTORY)

#################################################################################
# etags
#
if (NOT TB_SUBDIRECTORY)
  find_program (ETAGS etags)
  if (ETAGS)
    add_custom_target (tags
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND etags --language=c++ Demo/*/*.[ch]*
      COMMAND etags --language=c++ --append `find . -name '*.[ch]*'`
      )
    add_custom_target (etags DEPENDS tags)
  endif (ETAGS)
endif (NOT TB_SUBDIRECTORY)

#################################################################################
# CPack for TurboBadgerLib
#
if (NOT TB_BUILD_DEMO AND NOT TB_SUBDIRECTORY)
  set (CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
  set (CPACK_COMPONENTS_ALL library)

  set (CPACK_PACKAGE_NAME "TurboBadgerLib")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "TurboBadger UI Library")
  set (CPACK_PACKAGE_VERSION "${VERSION}${VERSION_BUILD}")
  set (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
  set (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
  set (CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
  set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
  set (CPACK_PACKAGE_URL "https://github.com/tesch1/turbobadger")
  set (CPACK_PACKAGE_VENDOR "nobody")
  set (CPACK_PACKAGE_CONTACT "tesch1@gmail.com")
  set (CPACK_PACKAGE_RELOCATABLE TRUE)
  set (CPACK_STRIP_FILES TRUE)
  set (CPACK_SOURCE_STRIP_FILES TRUE)

  # WIN32
  if (WIN32)
    set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-install-win32")
    set (CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set (CPACK_NSIS_URL_INFO_ABOUT "https://github.com/tesch1/turbobadger")
  endif (WIN32)

  # macOS
  if (APPLE AND NOT XCODE)
    set (CPACK_GENERATOR "DragNDrop")
    set (CPACK_BUNDLE_NAME "${CPACK_PACKAGE_NAME}")
  endif (APPLE AND NOT XCODE)

  # unix
  if (UNIX AND NOT APPLE)
    set (CPACK_GENERATOR "RPM;TGZ;DEB;STGZ")
    set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)
    set (CPACK_VERBATIM_VARIABLES TRUE) # fixes @ in rpm file names
    set (CPACK_RPM_PACKAGE_RELEASE 1)
    set (CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
  endif (UNIX AND NOT APPLE)

  include (CPack)

endif (NOT TB_BUILD_DEMO AND NOT TB_SUBDIRECTORY)

message (" CMAKE_BUILD_TYPE:    ${CMAKE_BUILD_TYPE}")
message (" TB_SUBDIRECTORY:     ${TB_SUBDIRECTORY}")
message (" TB_BUILD_DEMO:       ${TB_BUILD_DEMO}")
message (" TB_BUILD_SDL2:       ${TB_BUILD_SDL2}")
message (" TB_BUILD_GLFW:       ${TB_BUILD_GLFW}")
message (" TB_BUILD_FREETYPE:   ${TB_BUILD_FREETYPE}")
message (" TB_SYSTEM:           ${TB_SYSTEM}")
message (" TB_BACKEND:          ${TB_BACKEND}")
message (" TB_RENDERER:         ${TB_RENDERER}")
message (" TB_RENDERER_BATCHER: ${TB_RENDERER_BATCHER}")
message (" TB_FONT_RENDERER:    ${TB_FONT_RENDERER}")
message (" TB_FILE:             ${TB_FILE}")
message (" TB_CLIPBOARD:        ${TB_CLIPBOARD}")
message (" TB_IMAGE_LOADER:     ${TB_IMAGE_LOADER}")
