# Use project source dir for demo output. We have to set
# the release and debug specific defines too, for MSVC.
if(TB_DEMO_IN_SRC)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
endif()

include (ExternalProject)

if (TB_GET_GLFW)
  ExternalProject_Add (GLFWEXT
    PREFIX GLFWEXT
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.2.1"
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    )
  ExternalProject_Get_Property (GLFWEXT source_dir)
  set (GLFW_SRC_DIR "${source_dir}")
  set (GLFW_INCLUDE_DIRS "${source_dir}/include")
else (TB_GET_GLFW)
  set (GLFW_SRC_DIR "thirdparty/glfw/")
  set (GLFW_INCLUDE_DIRS "thirdparty/glfw/include/")
endif (TB_GET_GLFW)

if(TB_BUILD_DEMO_GLFW)
  if(CMAKE_SYSTEM MATCHES "Windows")
    add_definitions(-DGLFW_EXPOSE_NATIVE_WIN32)
    add_definitions(-DGLFW_EXPOSE_NATIVE_WGL)
    set(EXTRA_LIBS ${EXTRA_LIBS} winmm)
  endif(CMAKE_SYSTEM MATCHES "Windows")

  # Add sources
  set(LOCAL_SRCS "")
  aux_source_directory(./demo01 LOCAL_SRCS)
  set(LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/main.cpp
    ./platform/port_glfw.cpp
    ./platform/glfw_extra_linux.cpp
    ./platform/Application.cpp)
  include_directories(".")

  if (NOT TB_GET_GLFW)
    # Add glfw and gl libs on MacOSX
    if(APPLE)
      find_library(GLFW_LIBRARIES glfw )
      find_library(OPENGL_LIBRARY OpenGL )
      set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${GLFW_LIBRARIES})
      find_path(GLFW_INCLUDE_DIRS GLFW/glfw3.h)
    endif(APPLE)
  endif (NOT TB_GET_GLFW)

  if(NOT GLFW_LIBRARIES)
    # Configure glfw
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "just want glfw library" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "just want glfw library" FORCE)
    set(GLFW_CLIENT_LIBRARY opengl CACHE STRING "client to use for glfw" FOR) # glesv1, glesv2
    set(GLFW_INSTALL OFF CACHE BOOL "just want glfw library" FORCE)
    #set(GLFW_USE_OPTIMUS_HPG ON "just want glfw library")
    add_subdirectory(${GLFW_SRC_DIR})
    set(EXTRA_LIBS ${EXTRA_LIBS} glfw ${GLFW_LIBRARIES})
  endif(NOT GLFW_LIBRARIES)

  target_include_directories(TurboBadgerLib PUBLIC ${GLFW_INCLUDE_DIRS})
  include_directories(${GLFW_INCLUDE_DIRS})

  # Platform specific
  if(CMAKE_COMPILER_IS_MINGW)
    # Avoid dll dependencies by linking statically.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
  endif(CMAKE_COMPILER_IS_MINGW)

  add_executable(TurboBadgerDemo WIN32 ${LOCAL_SRCS})
  target_link_libraries(TurboBadgerDemo TurboBadgerLib ${EXTRA_LIBS})
  install(TARGETS TurboBadgerDemo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

endif(TB_BUILD_DEMO_GLFW)

if (TB_GET_SDL2)
  set (SDL_VERSION 2.0.8)
  set (SDL_VERSION_HASH 3800d705cef742c6a634f202c37f263f)
  set (SDL_VERSION 2.0.5)
  set (SDL_VERSION_HASH d4055424d556b4a908aa76fad63abd3c)
  message ("Building SDL2 Locally")
  set (SDL2_configure_args --disable-shared )
  # --disable-loadso --disable-cpuinfo --disable-atomic --disable-sdl-dlopen 
  if (APPLE)
    message ("Wouldn't you rather build the Framework?")
    set (SDL2_configure_args ${SDL2_configure_args} --without-x)
    set (SDL2_configure_args ${SDL2_configure_args} "CFLAGS=-mmacosx-version-min=10.9")
  endif ()
  ExternalProject_Add (SDL2EXT
    URL https://www.libsdl.org/release/SDL2-${SDL_VERSION}.tar.gz
    URL_HASH "MD5=${SDL_VERSION_HASH}"
    PREFIX SDL2
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${SDL2_configure_args} --prefix=<INSTALL_DIR>
    INSTALL_COMMAND make install
    )
  ExternalProject_Get_Property (SDL2EXT INSTALL_DIR)
  set (SDL2_INSTALL_DIR ${INSTALL_DIR})
  add_library (SDL2_LIBRARY STATIC IMPORTED GLOBAL)
  set_property (TARGET SDL2_LIBRARY PROPERTY IMPORTED_LOCATION ${SDL2_INSTALL_DIR}/lib/libSDL2.a)
  set (SDL2_LIBRARIES dl SDL2_LIBRARY dl pthread) # ${SDL2_LIBRARIES}
  set (SDL2_INCLUDE_DIRS "${SDL2_INSTALL_DIR}/include/SDL2")
  set (SDL2_INSTALL_DIR ${INSTALL_DIR})
endif(TB_GET_SDL2)

if(TB_BUILD_DEMO_SDL2)

  set(LOCAL_SRCS "")
  aux_source_directory(./demo01 LOCAL_SRCS)
  set(LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/main.cpp
    ./platform/port_sdl2.cpp
    ./platform/Application.cpp)
  include_directories(".")

  if(EMSCRIPTEN)
    # default skin resources
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file /tmp/resources@/resources")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file /tmp/demo01@Demo/demo01")
    # test files
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser.tb.txt@/")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser_included.tb.txt@/")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser_definitions.tb.txt@/")
  elseif(NOT TB_GET_SDL2 AND NOT EMSCRIPTEN)
    if(APPLE)
      find_package(OpenGL REQUIRED)
      find_library(SDL2_LIBRARIES SDL2)
      find_path(SDL2_INCLUDE_DIRS SDL.h)
      set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${SDL2_LIBRARIES} )
    else(APPLE)
      find_library(OPENGL_LIBRARY GL PATHS "/usr/lib64/nvidia" )
      find_library(SDL2_LIBRARIES SDL2 HINTS "$ENV{HOME}/local/lib")
      find_path(SDL2_INCLUDE_DIRS "SDL2/SDL.h"
        HINTS "$ENV{HOME}/local/include/"
        PATH_SUFFIXES "" "SDL2")
      set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${SDL2_LIBRARIES} )
    endif (APPLE)
    include_directories(${SDL2_INCLUDE_DIRS})
  endif(EMSCRIPTEN)

  # Platform specific
  if(CMAKE_COMPILER_IS_MINGW)
    # Avoid dll dependencies by linking statically.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
  endif(CMAKE_COMPILER_IS_MINGW)

  add_executable(TurboBadgerDemoSDL WIN32 ${LOCAL_SRCS})
  target_link_libraries(TurboBadgerDemoSDL TurboBadgerLib ${EXTRA_LIBS})
  install(TARGETS TurboBadgerDemoSDL
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

  if(EMSCRIPTEN)
    add_custom_command(TARGET TurboBadgerDemoSDL
      PRE_LINK
      COMMAND rm -rf /tmp/resources /tmp/demo01
      COMMAND rsync -av --exclude='*.psd' ${CMAKE_SOURCE_DIR}/resources /tmp/
      COMMAND rsync -av --include='*/' --include='*.txt' --include='*.png' --exclude='*' ${CMAKE_CURRENT_SOURCE_DIR}/demo01 /tmp )
  endif(EMSCRIPTEN)

endif(TB_BUILD_DEMO_SDL2)

