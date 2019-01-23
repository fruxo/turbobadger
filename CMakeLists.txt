project(TurboBadger CXX)
cmake_minimum_required(VERSION 3.1)
include(CheckCXXCompilerFlag)
set(CMAKE_CXX_STANDARD 11)

option(TB_BUILD_DEMO_GLFW "Build the Demo application. Depends on glfw." OFF)
option(TB_BUILD_DEMO_SDL2 "Build the Demo application. Depends on SDL2." OFF)
option(TB_DEMO_IN_SRC "Build the demo application to the source directory." OFF)

option(TB_DEBUG_POSTFIX "Add _d postfix for debug builds." OFF)
option(TB_GENERATE_CONFIG "Generate the tb_config.h file from provided parameters in cmake." ON)

set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Set build type")
set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
  "Debug" "Release" "RelWithDebInfo")

# Which types to generate for for Xcode and VS, default to Release
if (XCODE OR MSVS)
  set (CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo")
endif (XCODE OR MSVS)

# Configure file options
option(TB_FONT_RENDERER_TBBF "Enable to support TBBF fonts (Turbo Badger Bitmap Fonts)" ON)
option(TB_FONT_RENDERER_FREETYPE "Enable FreeType TB Font. Requires FreeType" OFF)
option(TB_FONT_RENDERER_STB "Enable to support truetype fonts using stb_truetype.h WARNING VERY UNSAFE" OFF)
option(TB_IMAGE_LOADER_STB "Enable to support image loading using stb_image.c" OFF)
option(TB_RENDERER_BATCHER "Enable to get TBRendererBatcher" ON)
option(TB_RENDERER_STUB "Enable stub renderer." OFF)
option(TB_RENDERER_GL "Enable renderer using OpenGL. This renderer depends on TB_RENDERER_BATCHER." OFF)
option(TB_RENDERER_GLES_1 "Enable renderer using OpenGL ES1. This renderer depends on TB_RENDERER_GL." OFF)
option(TB_RENDERER_GLES_2 "Enable renderer using OpenGL ES2. This renderer depends on TB_RENDERER_GL." OFF)
option(TB_RENDERER_GL3 "Enable renderer using OpenGL 3.2. This renderer depends on TB_RENDERER_GL." OFF)
option(TB_IMAGE "Enable support for TBImage, TBImageManager, TBImageWidget." ON)

# Needs further work
#option(TB_FILE_POSIX "" ON)
#option(TB_SYSTEM "" OFF)
#option(TB_TARGET "" OFF)
include(CMakeDependentOption)

# could try to auto-detect these
cmake_dependent_option(TB_SYSTEM_LINUX   "Set System to Linux" OFF
  "NOT TB_SYSTEM_WINDOWS;NOT TB_SYSTEM_ANDROID;NOT TB_SYSTEM_MACOSX" OFF)
cmake_dependent_option(TB_SYSTEM_ANDROID "Set System to Android" OFF
  "NOT TB_SYSTEM_LINUX;NOT TB_SYSTEM_WINDOWS;NOT TB_SYSTEM_MACOSX" OFF)
cmake_dependent_option(TB_SYSTEM_WINDOWS "Set System to Windows" OFF
  "NOT TB_SYSTEM_LINUX;NOT TB_SYSTEM_ANDROID;NOT TB_SYSTEM_MACOSX" OFF)
cmake_dependent_option(TB_SYSTEM_MACOSX "Set System to MacOSX" OFF
  "NOT TB_SYSTEM_LINUX;NOT TB_SYSTEM_ANDROID;NOT TB_SYSTEM_WINDOWS" OFF)

cmake_dependent_option(TB_CLIPBOARD_DUMMY "Clipboard Dummy" OFF
  "NOT TB_CLIPBOARD_GLFW; NOT TB_CLIPBOARD_WINDOWS; NOT TB_CLIPBOARD_SDL2" OFF)
cmake_dependent_option(TB_CLIPBOARD_GLFW "Clipboard GLFW" OFF
  "NOT TB_CLIPBOARD_DUMMY; NOT TB_CLIPBOARD_WINDOWS; NOT TB_CLIPBOARD_SDL2" OFF)
cmake_dependent_option(TB_CLIPBOARD_WINDOWS "Clipboard WINDOWS" OFF
  "NOT TB_CLIPBOARD_GLFW; NOT TB_CLIPBOARD_DUMMY; NOT TB_CLIPBOARD_SDL2" OFF)
cmake_dependent_option(TB_CLIPBOARD_SDL2 "Clipboard SDL2" OFF
  "NOT TB_CLIPBOARD_GLFW; NOT TB_CLIPBOARD_WINDOWS; NOT TB_CLIPBOARD_DUMMY" OFF)

# Runtime/subsystem configurations
if(TB_BUILD_DEMO_GLFW)
  set(TB_CLIPBOARD_GLFW ON)
  set(TB_IMAGE_LOADER_STB ON)
  if(TB_BUILD_DEMO_SDL2)
    message(FATAL_ERROR "Can not specify both TB_BUILD_DEMO_GLFW and TB_BUILD_DEMO_SDL")
  endif(TB_BUILD_DEMO_SDL2)
elseif(TB_BUILD_DEMO_SDL2)
  set(TB_CLIPBOARD_SDL2 ON)
  set(TB_SYSTEM_SDL ON)
  set(TB_IMAGE_LOADER_STB ON)
endif(TB_BUILD_DEMO_GLFW)

# System defaults
if(TB_SYSTEM_ANDROID)
  set(TB_SYSTEM_CONFIG "#define TB_SYSTEM_ANDROID")
  set(TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_DUMMY")
endif()
if(TB_SYSTEM_LINUX)
    set(TB_FILE_CONFIG "#define TB_FILE_POSIX")
    set(TB_SYSTEM_CONFIG "#define TB_SYSTEM_LINUX")
    set(TB_TARGET_CONFIG "#define TB_TARGET_LINUX")
    set(TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_GLFW")
endif()
if(TB_SYSTEM_MACOSX)
    set(TB_FILE_CONFIG "#define TB_FILE_POSIX")
    set(TB_SYSTEM_CONFIG "#define TB_SYSTEM_LINUX")
    set(TB_TARGET_CONFIG "#define TB_TARGET_MACOSX")
    set(TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_GLFW")
endif()
if(TB_SYSTEM_WINDOWS)
    set(TB_FILE_CONFIG "#define TB_FILE_POSIX")
    set(TB_SYSTEM_CONFIG "#define TB_SYSTEM_WINDOWS")
    set(TB_TARGET_CONFIG "#define TB_TARGET_WINDOWS")
endif()
if(TB_SYSTEM_SDL)
    set(TB_FILE_CONFIG "#define TB_FILE_POSIX")
    set(TB_SYSTEM_CONFIG "#define TB_SYSTEM_SDL")
    set(TB_TARGET_CONFIG "#define TB_TARGET_CUSTOM")
    set(TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_SDL2")
endif()

# Clipboard
if(TB_CLIPBOARD_DUMMY)
    set(TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_DUMMY")
endif()
if(TB_CLIPBOARD_GLFW)
    set(TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_GLFW")
endif()
if(TB_CLIPBOARD_WINDOWS)
    set(TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_WINDOWS")
endif()
if(TB_CLIPBOARD_SDL2)
    set(TB_CLIPBOARD_CONFIG "#define TB_CLIPBOARD_SDL2")
endif()

if(TB_FILE_POSIX)
    set(TB_FILE_CONFIG "#define TB_FILE_POSIX")
endif()
if(TB_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX _d)
endif()
if(TB_FONT_RENDERER_TBBF)
    set(TB_FONT_RENDERER_TBBF_CONFIG "#define TB_FONT_RENDERER_TBBF")
endif()
if(TB_FONT_RENDERER_FREETYPE)
    set(TB_FONT_RENDERER_FREETYPE_CONFIG "#define TB_FONT_RENDERER_FREETYPE")
    find_package(FreeType REQUIRED)
endif()
if(TB_FONT_RENDERER_STB)
    set(TB_FONT_RENDERER_STB_CONFIG "#define TB_FONT_RENDERER_STB")
endif()
if(TB_IMAGE_LOADER_STB)
    set(TB_IMAGE_LOADER_STB_CONFIG "#define TB_IMAGE_LOADER_STB")
endif()
if(TB_RENDERER_BATCHER)
    set(TB_RENDERER_BATCHER_CONFIG "#define TB_RENDERER_BATCHER")
endif()
if(TB_RENDERER_STUB)
    set(TB_RENDERER_STUB_CONFIG "#define TB_RENDERER_STUB")
endif()
if(TB_RENDERER_GL)
    set(TB_RENDERER_GL_CONFIG "#define TB_RENDERER_GL")
endif()
if(TB_RENDERER_GLES_1)
    set(TB_RENDERER_GLES_1_CONFIG "#define TB_RENDERER_GLES_1")
endif()
if(TB_RENDERER_GLES_2)
    set(TB_RENDERER_GLES_2_CONFIG "#define TB_RENDERER_GLES_2")
endif()
if(TB_RENDERER_GL3)
    set(TB_RENDERER_GL3_CONFIG "#define TB_RENDERER_GL3")
endif()
if(TB_IMAGE) 
    set(TB_IMAGE_CONFIG "#define TB_IMAGE")
endif()

configure_file(tb_config.h.in src/tb/tb_config.h)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-exceptions -fno-rtti -Wall -Wextra -Wno-unused-parameter")
endif()

if(APPLE)
 add_definitions(-DMACOSX)
endif(APPLE)

if(MSVC)
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
 add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
 # Remove exception handling
 STRING(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
 # Remove rtti
 STRING(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
endif(MSVC)

if(TB_SYSTEM_SDL)
  include(FindPkgConfig)
  pkg_search_module(SDL2 REQUIRED sdl2)
  set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIRS}")
  include_directories(${SDL2_INCLUDE_DIRS})
  link_directories(${SDL2_LIBRARY_DIRS})
endif(TB_SYSTEM_SDL)

if(EMSCRIPTEN)
 set(CMAKE_EXECUTABLE_SUFFIX ".html")
 #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Oz") # optional
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 ")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
endif(EMSCRIPTEN)

# Add TurboBadgerLib project
include_directories("${PROJECT_BINARY_DIR}/src/tb")
include_directories("${PROJECT_SOURCE_DIR}/src/tb")
add_subdirectory(src/tb)

# Add Demo project
if(TB_BUILD_DEMO_GLFW OR TB_BUILD_DEMO_SDL2)
 add_subdirectory(Demo)
endif(TB_BUILD_DEMO_GLFW OR TB_BUILD_DEMO_SDL2)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/tb
    DESTINATION include
    PATTERN "*.cpp" EXCLUDE
    PATTERN "*.mm" EXCLUDE
    PATTERN "*.txt" EXCLUDE
    PATTERN "test files" EXCLUDE
    PATTERN "*.h"
)

# Generate Doxygen documentation
configure_file(${CMAKE_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_BINARY_DIR}/doc/Doxyfile)
add_custom_target(doc 
  COMMAND doxygen ${CMAKE_BINARY_DIR}/doc/Doxyfile
#  COMMAND open doc/html/index.html
)
