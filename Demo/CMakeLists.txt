# Use project source dir for demo output. We have to set
# the release and debug specific defines too, for MSVC.
if(TB_DEMO_IN_SRC)
    SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
endif()

if(TB_BUILD_DEMO_GLFW)
if(CMAKE_SYSTEM MATCHES "Windows")
 add_definitions(-DGLFW_EXPOSE_NATIVE_WIN32)
 add_definitions(-DGLFW_EXPOSE_NATIVE_WGL)
 set(EXTRA_LIBS ${EXTRA_LIBS} winmm)
endif(CMAKE_SYSTEM MATCHES "Windows")

# Configure glfw
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)
#set(GLFW_USE_OPTIMUS_HPG ON)

# Add sources
set(LOCAL_SRCS "")
aux_source_directory(./demo01 LOCAL_SRCS)
  set(LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/port_glfw.cpp
    ./platform/glfw_extra_linux.cpp
    ./platform/Application.cpp)
include_directories(".")

# Add glfw and gl libs on MacOSX
if(APPLE)
  find_library(GLFW_LIBRARIES glfw )
  find_library(OPENGL_LIBRARY OpenGL )
  set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${GLFW_LIBRARIES})
  find_path(GLFW_INCLUDE_DIRS GLFW/glfw3.h)
  include_directories(${GLFW_INCLUDE_DIRS})
endif(APPLE)

if(NOT GLFW_LIBRARIES)
  target_include_directories(TurboBadgerLib PUBLIC "thirdparty/glfw/include")
  include_directories("thirdparty/glfw/include")
  add_subdirectory(thirdparty/glfw)
  set(EXTRA_LIBS ${EXTRA_LIBS} glfw)
endif(NOT GLFW_LIBRARIES)

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

if(TB_BUILD_DEMO_SDL)

  set(LOCAL_SRCS "")
  aux_source_directory(./demo01 LOCAL_SRCS)
  set(LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/port_sdl.cpp
    ./platform/Application.cpp)
  include_directories(".")

  if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Oz") # optional
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
    # default skin resources
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file /tmp/resources@/resources")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file /tmp/demo01@Demo/demo01")
    # test files
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ../src/tb/tests/test_tb_parser.tb.txt@/")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ../src/tb/tests/test_tb_parser_included.tb.txt@/")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ../src/tb/tests/test_tb_parser_definitions.tb.txt@/")
  elseif(APPLE)
    find_package(OpenGL REQUIRED)
    find_library(SDL2_LIBRARIES SDL2)
    find_path(SDL2_INCLUDE_DIRS SDL.h)
    set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${SDL2_LIBRARIES} )
  else(EMSCRIPTEN)
    set(_SEARCH_PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw # Fink
        /opt/local # DarwinPorts
        /opt/csw # Blastwave
        /opt
        $ENV{HOME}/local/include
    )

    find_library(OPENGL_LIBRARY GL PATHS "/usr/lib64/nvidia" )

    find_package(PkgConfig QUIET)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(SDL2 "")
    endif()
    find_path(SDL_INCLUDE_DIRS
        NAMES SDL.h
        HINTS ENV SDL2DIR
        PATH_SUFFIXES include include/SDL2 SDL2
        PATHS
            ${SDL2_INCLUDE_DIRS}
            ${_SEARCH_PATHS}
    )
    find_library(SDL_LIBRARIES
        NAMES sdl2 SDL2 ${SDL2_LIBRARIES}
        HINTS ENV SDL2DIR "$ENV{HOME}/local/lib"
        PATH_SUFFIXES lib64 lib
        PATHS
            ${SDL2_LIBRARY_DIRS}
            ${_SEARCH_PATHS}
    )
    include_directories(${SDL_INCLUDE_DIRS})
    set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${SDL_LIBRARIES} )
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
#      COMMAND rm -r /tmp/resources /tmp/demo01
      COMMAND rsync -av --exclude='*.psd' ../resources /tmp/resources
      COMMAND rsync -av --include='*/' --include='*.txt' --include='*.png' --exclude='*' demo01 /tmp/demo01 )
  endif(EMSCRIPTEN)

endif(TB_BUILD_DEMO_SDL)
