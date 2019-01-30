#
# CMake file for turbobadger dependencies
#

cmake_minimum_required (VERSION 2.8)
project (turbobadger_thirdparty NONE)

include (ExternalProject)
option (CMAKE_BUILD_TYPE "build type." "")
set (TB_THIRDPARTY_ROOT "${CMAKE_CURRENT_BINARY_DIR}")
get_directory_property (hasParent PARENT_DIRECTORY)

# quiet warning messages
message ("${CMAKE_CONFIGURATION_TYPES} ${ANDROID}")

if (TB_BUILD_GLFW)
  # Download glfw
  ExternalProject_Add (GLFWEXT
    PREFIX GLFWEXT
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.2.1"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON)
  ExternalProject_Get_Property (GLFWEXT source_dir)
  ExternalProject_Get_Property (GLFWEXT binary_dir)
  if (hasParent)
    set (GLFW_SRC_DIR "${source_dir}" PARENT_SCOPE)
    set (GLFW_INCLUDE_DIRS "${source_dir}/include" PARENT_SCOPE)

    # Configure glfw & add_subdirectory()
    set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_TESTS OFF CACHE BOOL "just want glfw library" FORCE)
    set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "just want glfw library" FORCE)
    set (GLFW_CLIENT_LIBRARY opengl CACHE STRING "client to use for glfw" FORCE) # glesv1, glesv2
    set (GLFW_INSTALL OFF CACHE BOOL "just want glfw library" FORCE)
    # set (GLFW_USE_OPTIMUS_HPG ON "just want glfw library")
    add_subdirectory (${source_dir} ${binary_dir})
    set (EXTRA_LIBS ${EXTRA_LIBS} glfw ${GLFW_LIBRARIES} PARENT_SCOPE)
  endif (hasParent)
endif (TB_BUILD_GLFW)

if (TB_BUILD_SDL2)
  set (SDL_VERSION 2.0.8)
  set (SDL_VERSION_HASH 3800d705cef742c6a634f202c37f263f)
  set (SDL_VERSION 2.0.5)
  set (SDL_VERSION_HASH d4055424d556b4a908aa76fad63abd3c)

  set (SDL2_configure_args --disable-shared)
  # --disable-loadso --disable-cpuinfo --disable-atomic --disable-sdl-dlopen

  if (APPLE)
    message ("Wouldn't you rather build the Framework?")
    set (SDL2_configure_args ${SDL2_configure_args} --without-x)
    set (SDL2_configure_args ${SDL2_configure_args} "CFLAGS=-mmacosx-version-min=10.9")
  endif ()

  ExternalProject_Add (SDL2EXT
    PREFIX SDL2EXT
    URL https://www.libsdl.org/release/SDL2-${SDL_VERSION}.tar.gz
    URL_HASH "MD5=${SDL_VERSION_HASH}"
    DOWNLOAD_DIR "$ENV{HOME}/Downloads"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${SDL2_configure_args} --prefix=<INSTALL_DIR>
    INSTALL_COMMAND make install)

  ExternalProject_Get_Property (SDL2EXT INSTALL_DIR)
  set (SDL2_INSTALL_DIR ${INSTALL_DIR})
  add_library (SDL2_LIBRARY STATIC IMPORTED GLOBAL)
  set_property (TARGET SDL2_LIBRARY PROPERTY IMPORTED_LOCATION ${SDL2_INSTALL_DIR}/lib/libSDL2.a)
  if (hasParent)
    set (SDL2_LIBRARIES dl SDL2_LIBRARY dl pthread PARENT_SCOPE)
    set (SDL2_INCLUDE_DIRS "${SDL2_INSTALL_DIR}/include/SDL2" PARENT_SCOPE)
    set (SDL2_INSTALL_DIR ${INSTALL_DIR} PARENT_SCOPE)
  endif (hasParent)
endif (TB_BUILD_SDL2)
