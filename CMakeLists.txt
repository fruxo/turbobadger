project(TurboBadger)
cmake_minimum_required(VERSION 2.8)

option(TB_BUILD_DEMO "Build the Demo application. Depends on glfw." ON)
option(TB_DEMO_IN_SRC "Build the demo application to the source directory" ON)
option(TB_DEBUG_POSTFIX "Add _d postfix for debug builds" OFF)
option(TB_GENERATE_CONFIG "Generate the tb_config.h file from provided parameters in cmake. Default Off" OFF)

# Configure file options
option(TB_FONT_RENDERER_TBBF "Enable to support TBBF fonts (Turbo Badger Bitmap Fonts)" ON)
option(TB_FONT_RENDERER_FREETYPE "Enable FreeType TB Font. Requires FreeType" OFF)
option(TB_FONT_RENDERER_STB "Enable to support truetype fonts using stb_truetype.h WARNING VERY UNSAFE" OFF)
option(TB_IMAGE_LOADER_STB "Enable to support image loading using stb_image.c" OFF)
option(TB_RENDERER_BATCHER "Enable to get TBRendererBatcher" ON)
option(TB_RENDERER_GL "Enable renderer using OpenGL. This renderer depends on TB_RENDERER_BATCHER." ON)
option(TB_RENDERER_GLES_1 "Enable renderer using OpenGL ES. This renderer depends on TB_RENDERER_GL." OFF)
option(TB_IMAGE "Enable support for TBImage, TBImageManager, TBImageWidget." ON)

# Runtime/subsystem configurations
# Needs further work
#option(TB_FILE_POSIX "" ON)
#option(TB_SYSTEM "" OFF)
#option(TB_TARGET "" OFF)


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
    set(TB_IMAGE_LOADER_STB_CONFIG "#define TB_FONT_RENDERER_STB")
endif()
if(TB_RENDERER_BATCHER)
    set(TB_RENDERER_BATCHER_CONFIG "#define TB_RENDERER_BATCHER")
endif()
if(TB_RENDERER_GL)
    set(TB_RENDERER_GL_CONFIG "#define TB_RENDERER_GL")
endif()
if(TB_RENDERER_GLES_1)
    set(TB_RENDERER_GLES_1_CONFIG "#define TB_RENDERER_GLES_1")
endif()
if(TB_IMAGE) 
    set(TB_IMAGE_CONFIG "#define TB_IMAGE")
endif()

configure_file(tb_config.h.in src/tb/config.h)
 
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -fno-exceptions -fno-rtti -Wall -Wextra -Wno-unused-parameter")
endif()

if(APPLE)
 add_definitions(-DMACOSX)
 add_definitions(-Dnullptr=0)
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

# Add TurboBadgerLib project
include_directories("${PROJECT_SOURCE_DIR}/src/tb")
add_subdirectory(src/tb)

# Add Demo project
if(TB_BUILD_DEMO)
 add_subdirectory(Demo)
endif(TB_BUILD_DEMO)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/tb
    DESTINATION include
    PATTERN "*.cpp" EXCLUDE
    PATTERN "*.mm" EXCLUDE
    PATTERN "*.txt" EXCLUDE
    PATTERN "test files" EXCLUDE
    PATTERN "*.h"
)

# Generate Doxygen documentation
add_custom_target (doc 
  COMMAND doxygen doc/Doxyfile
  COMMAND open doc/html/index.html
)