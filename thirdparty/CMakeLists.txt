#
# CMake file for turbobadger dependencies
#

cmake_minimum_required (VERSION 3.4.3)
project (turbobadger_thirdparty NONE)

include (ExternalProject)
option (CMAKE_BUILD_TYPE "build type." "")
set (TB_THIRDPARTY_ROOT "${CMAKE_CURRENT_BINARY_DIR}")
get_directory_property (hasParent PARENT_DIRECTORY)

if (NOT WIN32)
  set (DOWNLOAD_DIR "$ENV{HOME}/Downloads")
else (NOT WIN32)
  set (DOWNLOAD_DIR "C:/Downloads")
endif (NOT WIN32)

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
elseif (EMSCRIPTEN AND TB_CLIPBOARD_GLFW)
  add_library (glfw INTERFACE IMPORTED GLOBAL)
  set_property (TARGET glfw APPEND PROPERTY INTERFACE_COMPILE_OPTIONS -s USE_GLFW=3)
  set_property (TARGET glfw APPEND PROPERTY INTERFACE_LINK_OPTIONS    -s USE_GLFW=3)
endif (TB_BUILD_GLFW)

if (TB_BUILD_SDL2)
  set (SDL_VERSION 2.0.10)
  set (SDL_VERSION_HASH 5a2114f2a6f348bdab5bf52b994811db)
  set (SDL_VERSION 2.0.5)
  set (SDL_VERSION_HASH d4055424d556b4a908aa76fad63abd3c)

  ExternalProject_Add (sdl2X
    PREFIX sdl2X
    URL https://www.libsdl.org/release/SDL2-${SDL_VERSION}.tar.gz
    URL_HASH "MD5=${SDL_VERSION_HASH}"
    DOWNLOAD_DIR "${DOWNLOAD_DIR}"
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON)
  ExternalProject_Get_Property (sdl2X source_dir)

  if (hasParent)
    set (SDL_STATIC     ON  CACHE BOOL "" FORCE)
    set (SDL_STATIC_PIC ON  CACHE BOOL "" FORCE)
    set (SDL_SHARED     OFF CACHE BOOL "" FORCE) 
    set (SDL_RENDER     OFF CACHE BOOL "" FORCE)
   add_subdirectory ("${source_dir}" "${CMAKE_CURRENT_BINARY_DIR}/sdl2X_binary_dir")
    add_library (SDL2-interface INTERFACE IMPORTED GLOBAL)
    target_link_libraries (SDL2-interface INTERFACE SDL2-static)
    if (SDL_VERSION VERSION_LESS "2.0.10")
      set_property (TARGET SDL2-interface APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_BINARY_DIR}/sdl2X_binary_dir/include;${source_dir}/include")
    endif (SDL_VERSION VERSION_LESS "2.0.10")

    if (WIN32)
      target_link_libraries (SDL2-interface INTERFACE SDL2main)
    endif (WIN32)
  endif (hasParent)
elseif (EMSCRIPTEN AND TB_SYSTEM_SDL2)
  add_library (SDL2-interface INTERFACE IMPORTED GLOBAL)
  set_property (TARGET SDL2-interface APPEND PROPERTY INTERFACE_COMPILE_OPTIONS -s USE_SDL=2)
  set_property (TARGET SDL2-interface APPEND PROPERTY INTERFACE_LINK_OPTIONS    -s USE_SDL=2)
endif (TB_BUILD_SDL2)

