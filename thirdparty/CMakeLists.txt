#
# CMake file for turbobadger dependencies
#

cmake_minimum_required (VERSION 3.4.3)
project (turbobadger_thirdparty NONE)

include (ExternalProject)
get_directory_property (hasParent PARENT_DIRECTORY)

if (NOT WIN32)
  set (DOWNLOAD_DIR "$ENV{HOME}/Downloads")
else (NOT WIN32)
  set (DOWNLOAD_DIR "C:/Downloads")
endif (NOT WIN32)

# quiet warning messages
message ("${CMAKE_CONFIGURATION_TYPES} ${ANDROID}")

if (hasParent)
  #
  # setup search paths
  #
  if (IOS)
    set (CMAKE_FRAMEWORK_PATH "${CMAKE_SOURCE_DIR}/Frameworks.iOS")
  else ()
    set (CMAKE_FRAMEWORK_PATH "${CMAKE_SOURCE_DIR}/Frameworks.macOS")
  endif ()
endif (hasParent)

if (TB_BUILD_FREETYPE)
  #
  # freetype
  #
  ExternalProject_Add (freetypeX
    PREFIX freetypeX
    URL http://download.savannah.gnu.org/releases/freetype/freetype-2.9.1.tar.gz
    URL_HASH MD5=3adb0e35d3c100c456357345ccfa8056
    #URL http://download.savannah.gnu.org/releases/freetype/freetype-2.8.1.tar.gz
    #URL_HASH MD5=c947b6b1c513e814cc9d7617a44bc6cf
    DOWNLOAD_DIR ${DOWNLOAD_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON )
  ExternalProject_Get_Property (freetypeX source_dir)
  if (hasParent)
    set (FREETYPE_SRC_DIR "${source_dir}")
    message (" FREETYPE_SRC_DIR: ${FREETYPE_SRC_DIR}")
    # freetype config variables
    set (FT_WITH_ZLIB OFF CACHE STRING "" FORCE)
    set (FT_WITH_BZIP2 OFF CACHE STRING "" FORCE)
    set (FT_WITH_PNG OFF CACHE STRING "" FORCE)
    set (FT_WITH_HARFBUZZ OFF CACHE STRING "" FORCE)
    set (CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON CACHE STRING "" FORCE)
    set (CMAKE_DISABLE_FIND_PACKAGE_BZip2 ON CACHE STRING "" FORCE)
    set (CMAKE_DISABLE_FIND_PACKAGE_PNG ON CACHE STRING "" FORCE)
    set (CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE STRING "" FORCE)
    set (FREETYPE_NO_DIST ON CACHE STRING "" FORCE)
    add_subdirectory ("${FREETYPE_SRC_DIR}"
      "${CMAKE_CURRENT_BINARY_DIR}/freetype_build"
      EXCLUDE_FROM_ALL)
    set_property (TARGET freetype APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<TARGET_PROPERTY:freetype,INCLUDE_DIRECTORIES>)
  endif (hasParent)
elseif (EMSCRIPTEN AND TB_FONT_RENDERER STREQUAL FREETYPE)
  add_library (freetype INTERFACE IMPORTED GLOBAL)
  set_property (TARGET freetype APPEND PROPERTY INTERFACE_COMPILE_OPTIONS -s USE_FREETYPE=1)
  set_property (TARGET freetype APPEND PROPERTY INTERFACE_LINK_OPTIONS    -s USE_FREETYPE=1)
endif (TB_BUILD_FREETYPE)

if (TB_BUILD_GLFW)
  # Download glfw
  ExternalProject_Add (GLFWEXT
    PREFIX GLFWEXT
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.3"
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
elseif (TB_BACKEND STREQUAL GLFW)
  #add_library (glfw INTERFACE IMPORTED GLOBAL)
  if (EMSCRIPTEN)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3" CACHE STRING BLAH FORCE)
    #set_property (TARGET glfw APPEND PROPERTY INTERFACE_COMPILE_OPTIONS -s USE_GLFW=3)
    #set_property (TARGET glfw APPEND PROPERTY INTERFACE_LINK_OPTIONS    -s USE_GLFW=3)
  else ()
    find_package (glfw3 REQUIRED)
    #add_library (glfw ALIIAS glfw3)
  endif (EMSCRIPTEN)
endif (TB_BUILD_GLFW)

if (TB_BUILD_SDL2)
  set (SDL_VERSION 2.0.5)
  set (SDL_VERSION_HASH d4055424d556b4a908aa76fad63abd3c)
  set (SDL_VERSION 2.0.10)
  set (SDL_VERSION_HASH 5a2114f2a6f348bdab5bf52b994811db)

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
    set (SDL_TEST       OFF CACHE BOOL "" FORCE) 
    set (SDL_RENDER     OFF CACHE BOOL "" FORCE)
    #set (SDL_HAPTIC     OFF CACHE BOOL "" FORCE)
    #set (SDL_JOYSTICK   OFF CACHE BOOL "" FORCE)
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
elseif (TB_BACKEND STREQUAL SDL2)
  #
  # Not building SDL2, so try to find it
  #
  add_library (SDL2-interface INTERFACE IMPORTED GLOBAL)
  if (EMSCRIPTEN)
    #set_property (TARGET SDL2-interface APPEND PROPERTY INTERFACE_COMPILE_OPTIONS -s USE_SDL=2)
    #set_property (TARGET SDL2-interface APPEND PROPERTY INTERFACE_LINK_OPTIONS    -s USE_SDL=2)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2" CACHE STRING BLAH FORCE)
  else ()
    message ("Looking for SDL2 with find_package()...")
    find_package (SDL2 2.0.10)
    if (NOT SDL2)
      message ("Looking for SDL2 with find_library()...")
      find_library (SDL2_LIBRARY SDL2)
      find_path (SDL2_INCLUDE_DIR "SDL.h" PATH_SUFFIXES "SDL2")
      message ("SDL2_LIBRARY: ${SDL2_LIBRARY}, SDL2_INCLUDE_DIR: ${SDL2_INCLUDE_DIR}")
      if (SDL2_LIBRARY AND SDL2_INCLUDE_DIR)
        add_library (SDL2 INTERFACE IMPORTED GLOBAL)
        set_property (TARGET SDL2 APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${SDL2_LIBRARY})
        set_property (TARGET SDL2 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIR})
        set (SDL2 TRUE)
      endif ()
      message ("Found: '${SDL2}', '${SDL2_INCLUDE_DIR}' ...")
    endif (NOT SDL2)
    if (NOT SDL2)
      message ("Looking for SDL2 with pkg_search_module()...")
      include (FindPkgConfig)
      pkg_search_module (SDL2 REQUIRED sdl2)
      #set_property (TARGET SDL2-interface APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${SDL2_LIBRARY})
      #set_property (TARGET SDL2-interface APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIR})
    endif (NOT SDL2)
    if (NOT SDL2)
      message (FATAL_ERROR "Unable to find SDL2: ${SDL2}, ${SDL2_FOUND}")
    endif ()
    set_property (TARGET SDL2-interface APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS SDL_FRAMEWORK=1)
    target_link_libraries (SDL2-interface INTERFACE SDL2)
  endif (EMSCRIPTEN)
endif (TB_BUILD_SDL2)

if (WIN32)
  ExternalProject_Add (GLEWX
    PREFIX GLEWX
    URL "https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip"
    URL_HASH MD5=32a72e6b43367db8dbea6010cd095355
    DOWNLOAD_DIR ${DOWNLOAD_DIR}
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    )
  ExternalProject_Get_Property (GLEWX source_dir)
  if (hasParent)
    add_library (GLEW INTERFACE IMPORTED GLOBAL)
    set_property (TARGET GLEW APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${source_dir}/include")
    set_property (TARGET GLEW APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${source_dir}/lib/Release/Win32/glew32s.lib")
    set_property (TARGET GLEW APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS GLEW_STATIC=1)
  endif (hasParent)
endif (WIN32)
