project (TurboBadger CXX)
cmake_minimum_required (VERSION 3.4.3)
include (CheckCXXCompilerFlag)
set (CMAKE_CXX_STANDARD 14)

option (TB_BUILD_DEMO_GLFW "Build the Demo application. Depends on glfw." OFF)
option (TB_BUILD_DEMO_SDL2 "Build the Demo application. Depends on SDL2." OFF)
option (TB_BUILD_DEMO_ANDROID "Build the Android Demo application. Depends on Android." OFF)
option (TB_BUILD_GLFW "Download and build the GLFW library." OFF)
option (TB_BUILD_SDL2 "Download and build the SDL2 library." OFF)
option (TB_DEBUG_POSTFIX "Add _d postfix for debug builds." OFF)
option (TB_GENERATE_CONFIG "Generate the tb_config.h file from provided parameters in cmake." ON)

set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Set build type")
set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")

# Which types to generate for for Xcode and VS, default to Release
if (XCODE OR MSVS)
  set (CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo")
endif (XCODE OR MSVS)

# Configure file options
option (TB_FONT_RENDERER_TBBF "Enable to support TBBF fonts (Turbo Badger Bitmap Fonts)" ON)
option (TB_FONT_RENDERER_FREETYPE "Enable FreeType TB Font. Requires FreeType" OFF)
option (TB_FONT_RENDERER_STB "Enable to support truetype fonts using stb_truetype.h WARNING VERY UNSAFE" OFF)
option (TB_IMAGE_LOADER_STB "Enable to support image loading using stb_image.c" OFF)
option (TB_RENDERER_BATCHER "Enable to get TBRendererBatcher" ON)
option (TB_RENDERER_GL "Enable renderer using OpenGL. This renderer depends on TB_RENDERER_BATCHER." OFF)
option (TB_RENDERER_GLES_1 "Enable renderer using OpenGL ES1. This renderer depends on TB_RENDERER_GL." OFF)
option (TB_RENDERER_GLES_2 "Enable renderer using OpenGL ES2. This renderer depends on TB_RENDERER_GL." OFF)
option (TB_RENDERER_GL3 "Enable renderer using OpenGL 3.2. This renderer depends on TB_RENDERER_GL." OFF)
option (TB_RENDERER_SDL2 "Enable pure SDL2 renderer." OFF)
option (TB_IMAGE "Enable support for TBImage, TBImageManager, TBImageWidget." ON)

# Needs further work option (TB_FILE_POSIX "" ON) option (TB_SYSTEM "" OFF) option (TB_TARGET "" OFF)
include (CMakeDependentOption)

# Runtime/subsystem configurations
if (TB_BUILD_DEMO_GLFW)
  set (TB_BUILD_GLFW ON)
  set (TB_CLIPBOARD_GLFW ON)
  set (TB_IMAGE_LOADER_STB ON)
  if (TB_BUILD_DEMO_SDL2 OR TB_BUILD_DEMO_ANDROID)
    message (FATAL_ERROR "Can not specify both TB_BUILD_DEMO_GLFW and TB_BUILD_DEMO_SDL2")
  endif (TB_BUILD_DEMO_SDL2)
elseif (TB_BUILD_DEMO_SDL2)
  if (NOT EMSCRIPTEN)
    set (TB_BUILD_SDL2 ON)
  endif (NOT EMSCRIPTEN)
  set (TB_CLIPBOARD_SDL2 ON)
  set (TB_SYSTEM_SDL2 ON)
  set (TB_IMAGE_LOADER_STB ON)
  if (TB_BUILD_DEMO_ANDROID)
    message (FATAL_ERROR "Can not specify both TB_BUILD_DEMO_SDL2 and TB_BUILD_DEMO_ANDROID")
  endif (TB_BUILD_DEMO_SDL2)
elseif (TB_BUILD_DEMO_ANDROID)
  set (TB_CLIPBOARD_DUMMY ON)
  set (TB_SYSTEM_ANDROID ON)
  set (TB_IMAGE_LOADER_STB ON)
endif (TB_BUILD_DEMO_GLFW)

# could try to auto-detect these
cmake_dependent_option (TB_SYSTEM_LINUX "Set System to Linux" ON
  "NOT TB_SYSTEM_WINDOWS;NOT TB_SYSTEM_ANDROID;NOT TB_SYSTEM_MACOSX;NOT TB_SYSTEM_SDL2" OFF)
cmake_dependent_option (TB_SYSTEM_ANDROID "Set System to Android" ON
  "NOT TB_SYSTEM_LINUX;NOT TB_SYSTEM_WINDOWS;NOT TB_SYSTEM_MACOSX;NOT TB_SYSTEM_SDL2" OFF)
cmake_dependent_option (TB_SYSTEM_WINDOWS "Set System to Windows" ON
  "NOT TB_SYSTEM_LINUX;NOT TB_SYSTEM_ANDROID;NOT TB_SYSTEM_MACOSX;NOT TB_SYSTEM_SDL2" OFF)
cmake_dependent_option (TB_SYSTEM_MACOSX "Set System to MacOSX" ON
  "NOT TB_SYSTEM_LINUX;NOT TB_SYSTEM_ANDROID;NOT TB_SYSTEM_WINDOWS;NOT TB_SYSTEM_SDL2" OFF)
cmake_dependent_option (TB_SYSTEM_SDL2 "Set System to SDL2" ON
  "NOT TB_SYSTEM_LINUX;NOT TB_SYSTEM_ANDROID;NOT TB_SYSTEM_WINDOWS;NOT TB_SYSTEM_MACOSX" OFF)

cmake_dependent_option (TB_CLIPBOARD_DUMMY "Clipboard Dummy" ON
  "NOT TB_CLIPBOARD_GLFW; NOT TB_CLIPBOARD_WINDOWS; NOT TB_CLIPBOARD_SDL2" OFF)
cmake_dependent_option (TB_CLIPBOARD_GLFW "Clipboard GLFW" ON
  "NOT TB_CLIPBOARD_DUMMY; NOT TB_CLIPBOARD_WINDOWS; NOT TB_CLIPBOARD_SDL2" OFF)
cmake_dependent_option (TB_CLIPBOARD_WINDOWS "Clipboard WINDOWS" ON
  "NOT TB_CLIPBOARD_GLFW; NOT TB_CLIPBOARD_DUMMY; NOT TB_CLIPBOARD_SDL2" OFF)
cmake_dependent_option (TB_CLIPBOARD_SDL2 "Clipboard SDL2" ON
  "NOT TB_CLIPBOARD_GLFW; NOT TB_CLIPBOARD_WINDOWS; NOT TB_CLIPBOARD_DUMMY" OFF)

# System defaults
if (TB_SYSTEM_ANDROID)
  set (TB_SYSTEM_CONFIG "#define TB_SYSTEM_ANDROID")
  set (TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_DUMMY")
endif ()
if (TB_SYSTEM_LINUX)
  set (TB_FILE_CONFIG "#define TB_FILE_POSIX")
  set (TB_SYSTEM_CONFIG "#define TB_SYSTEM_LINUX")
  set (TB_TARGET_CONFIG "#define TB_TARGET_LINUX")
  set (TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_GLFW")
endif ()
if (TB_SYSTEM_MACOSX)
  set (TB_FILE_CONFIG "#define TB_FILE_POSIX")
  set (TB_SYSTEM_CONFIG "#define TB_SYSTEM_LINUX")
  set (TB_TARGET_CONFIG "#define TB_TARGET_MACOSX")
  set (TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_GLFW")
endif ()
if (TB_SYSTEM_WINDOWS)
  set (TB_FILE_CONFIG "#define TB_FILE_POSIX")
  set (TB_SYSTEM_CONFIG "#define TB_SYSTEM_WINDOWS")
  set (TB_TARGET_CONFIG "#define TB_TARGET_WINDOWS")
endif ()
if (TB_SYSTEM_SDL2)
  set (TB_FILE_CONFIG "#define TB_FILE_POSIX")
  set (TB_SYSTEM_CONFIG "#define TB_SYSTEM_SDL2")
  set (TB_TARGET_CONFIG "#define TB_TARGET_CUSTOM")
  set (TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_SDL2")
endif ()

# Clipboard
if (TB_CLIPBOARD_DUMMY)
  set (TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_DUMMY")
endif ()
if (TB_CLIPBOARD_GLFW)
  set (TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_GLFW")
endif ()
if (TB_CLIPBOARD_WINDOWS)
  set (TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_WINDOWS")
endif ()
if (TB_CLIPBOARD_SDL2)
  set (TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_SDL2")
endif ()

if (TB_FILE_POSIX)
  set (TB_FILE_CONFIG "#define TB_FILE_POSIX")
endif ()
if (TB_DEBUG_POSTFIX)
  set (CMAKE_DEBUG_POSTFIX _d)
endif ()
if (TB_FONT_RENDERER_TBBF)
  set (TB_FONT_RENDERER_TBBF_CONFIG "#define TB_FONT_RENDERER_TBBF")
endif ()
if (TB_FONT_RENDERER_FREETYPE)
  set (TB_FONT_RENDERER_FREETYPE_CONFIG "#define TB_FONT_RENDERER_FREETYPE")
  find_package (FreeType REQUIRED)
endif ()
if (TB_FONT_RENDERER_STB)
  set (TB_FONT_RENDERER_STB_CONFIG "#define TB_FONT_RENDERER_STB")
endif ()
if (TB_IMAGE_LOADER_STB)
  set (TB_IMAGE_LOADER_STB_CONFIG "#define TB_IMAGE_LOADER_STB")
endif ()
if (TB_RENDERER_BATCHER)
  set (TB_RENDERER_BATCHER_CONFIG "#define TB_RENDERER_BATCHER")
endif ()
if (TB_RENDERER_STUB)
  set (TB_RENDERER_STUB_CONFIG "#define TB_RENDERER_STUB")
endif ()
if (TB_RENDERER_GLES_1)
  set (TB_RENDERER_GL ON)
  set (TB_RENDERER_GLES_1_CONFIG "#define TB_RENDERER_GLES_1")
endif ()
if (TB_RENDERER_GLES_2)
  set (TB_RENDERER_GL ON)
  set (TB_RENDERER_GLES_2_CONFIG "#define TB_RENDERER_GLES_2")
endif ()
if (TB_RENDERER_GL3)
  set (TB_RENDERER_GL ON)
  set (TB_RENDERER_GL3_CONFIG "#define TB_RENDERER_GL3")
endif ()
if (TB_RENDERER_GL)
  set (TB_RENDERER_GL_CONFIG "#define TB_RENDERER_GL")
endif ()
if (TB_IMAGE)
  set (TB_IMAGE_CONFIG "#define TB_IMAGE")
endif ()

if (TB_GENERATE_CONFIG)
  configure_file (tb_config.h.in src/tb/tb_config.h)
endif (TB_GENERATE_CONFIG)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-exceptions -fno-rtti -Wall -Wextra -Wno-unused-parameter")
endif ()

if (APPLE)
  add_definitions (-DMACOSX)
endif (APPLE)

if (MSVC)
  add_definitions (-D_CRT_SECURE_NO_WARNINGS)
  add_definitions (-D_CRT_NONSTDC_NO_DEPRECATE)
  # Remove exception handling
  string (REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  # Remove rtti
  string (REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
endif (MSVC)

if (EMSCRIPTEN)
  set (CMAKE_EXECUTABLE_SUFFIX ".html")
  # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Oz") # optional
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 ")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s FULL_ES2=1")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EVAL_CTORS=1")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
  set (TB_SYSTEM_SDL2 ON)
endif (EMSCRIPTEN)

#
# Look for libraries
if (TB_CLIPBOARD_GLFW)
  if (NOT TB_BUILD_GLFW)
    # Add glfw and gl libs on MacOSX
    if (APPLE)
      find_library (GLFW_LIBRARIES glfw)
      set (EXTRA_LIBS ${EXTRA_LIBS} ${GLFW_LIBRARIES})
      find_path (GLFW_INCLUDE_DIRS GLFW/glfw3.h)
    else (APPLE)
      find_library (GLFW_LIBRARIES glfw)
    endif (APPLE)
  endif (NOT TB_BUILD_GLFW)
endif (TB_CLIPBOARD_GLFW)

if (TB_SYSTEM_SDL2)
  if (NOT TB_BUILD_SDL2 AND NOT EMSCRIPTEN)
    include (FindPkgConfig)
    pkg_search_module (SDL2 REQUIRED sdl2)
    set (SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIRS}")
    include_directories (${SDL2_INCLUDE_DIRS})
    link_directories (${SDL2_LIBRARY_DIRS})
    if (APPLE)
      find_library (SDL2_LIBRARIES SDL2)
      find_path (SDL2_INCLUDE_DIRS SDL.h)
      set (EXTRA_LIBS ${EXTRA_LIBS} ${SDL2_LIBRARIES})
    else (APPLE)
      find_library (SDL2_LIBRARIES SDL2)
      find_path (SDL2_INCLUDE_DIRS "SDL2/SDL.h" PATH_SUFFIXES "" "SDL2")
    endif (APPLE)
  endif (NOT TB_BUILD_SDL2 AND NOT EMSCRIPTEN)
endif (TB_SYSTEM_SDL2)

if (TB_RENDERER_GL AND NOT ANDROID)
  find_package (OpenGL REQUIRED)
  set (EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY})
endif ()

#
# Add thirdparty dependencies
#
file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/thirdparty")

# Generator name
if (NOT "${CMAKE_EXTRA_GENERATOR}" STREQUAL "")
  set (GENERATOR_STRING "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else ()
  set (GENERATOR_STRING "${CMAKE_GENERATOR}")
endif ()

# Configure the thirdparty build directory
execute_process (
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/thirdparty"
  COMMAND ${CMAKE_COMMAND} -G${GENERATOR_STRING} -DANDROID=${ANDROID} -DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -DTB_BUILD_GLFW=${TB_BUILD_GLFW} -DTB_BUILD_SDL2=${TB_BUILD_SDL2} ${CMAKE_SOURCE_DIR}/thirdparty
  RESULT_VARIABLE THIRDPARTY_CONFIG_RESULT
)
if (THIRDPARTY_CONFIG_RESULT)
  message (FATAL_ERROR "Configuring dependencies failed: ${THIRDPARTY_CONFIG_RESULT}")
endif ()

# Build the thirdparty directory
execute_process (
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/thirdparty"
  COMMAND "${CMAKE_COMMAND}" --build .
  RESULT_VARIABLE THIRDPARTY_BUILD_RESULT
  )
if (THIRDPARTY_BUILD_RESULT)
  message (FATAL_ERROR "Building dependencies failed: ${THIRDPARTY_BUILD_RESULT}")
endif ()

# Include the now-built thirdparty dependencies
add_subdirectory (thirdparty)

# Add TurboBadgerLib project
include_directories ("${PROJECT_BINARY_DIR}/src/tb")
include_directories ("${PROJECT_SOURCE_DIR}/src/tb")
add_subdirectory (src/tb)

# Add Demo project
if (TB_BUILD_DEMO_GLFW OR TB_BUILD_DEMO_SDL2)
  add_subdirectory (Demo)
endif (TB_BUILD_DEMO_GLFW OR TB_BUILD_DEMO_SDL2)

# Android demo
if (TB_BUILD_DEMO_ANDROID)
  message ("CMAKE_ANDROID_ASSETS_DIRECTORIES: ${CMAKE_ANDROID_ASSETS_DIRECTORIES}")
  list (APPEND CMAKE_ANDROID_ASSETS_DIRECTORIES "${CMAKE_SOURCE_DIR}/resources/language")
  add_definitions (-DTB_RUNTIME_DEBUG_INFO=1)
  add_library (TurboBadger SHARED
    ./DemoAndroid2/app/src/main/jni/jni_glue.cpp
    ./DemoAndroid2/app/src/main/jni/App.cpp
#    $<TARGET_OBJECTS:TurboBadgerLib>
    )
  #set_target_properties (TurboBadger ANDROID_ASSETS_DIRECTORIES "${CMAKE_SOURCE_DIR}/resources/default_skin")
  target_link_libraries (TurboBadger TurboBadgerLib)
  #add_dependencies (TurboBadger )
endif (TB_BUILD_DEMO_ANDROID)

install (
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/tb
  DESTINATION include
  PATTERN "*.cpp" EXCLUDE
  PATTERN "*.mm" EXCLUDE
  PATTERN "*.txt" EXCLUDE
  PATTERN "test files" EXCLUDE
  PATTERN "*.h"
  )

# Generate Doxygen documentation
configure_file (${CMAKE_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_BINARY_DIR}/doc/Doxyfile)
add_custom_target (doc
  COMMAND doxygen ${CMAKE_BINARY_DIR}/doc/Doxyfile
  # COMMAND open doc/html/index.html
  )
