# Use project source dir for demo output. We have to set
# the release and debug specific defines too, for MSVC.
if(TB_DEMO_IN_SRC)
    SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
endif()

if (TB_BUILD_DEMO_GLFW)
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
  set (LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/port_glfw.cpp
    ./platform/glfw_extra_linux.cpp
    ./platform/Application.cpp)
include_directories(".")

# Add glfw and gl libs on MacOSX
if(APPLE)
  find_library (GLFW_LIBRARIES glfw )
  find_library (OPENGL_LIBRARY OpenGL )
  set (EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${GLFW_LIBRARIES})
  find_path (GLFW_INCLUDE_DIRS GLFW/glfw3.h)
  include_directories(${GLFW_INCLUDE_DIRS})
endif(APPLE)

if (NOT GLFW_LIBRARIES)
  target_include_directories(TurboBadgerLib PUBLIC "thirdparty/glfw/include")
  include_directories("thirdparty/glfw/include")
  add_subdirectory(thirdparty/glfw)
  set (EXTRA_LIBS ${EXTRA_LIBS} glfw)
endif (NOT GLFW_LIBRARIES)

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

if (TB_BUILD_DEMO_SDL2)

  set(LOCAL_SRCS "")
  aux_source_directory(./demo01 LOCAL_SRCS)
  set (LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/port_sdl2.cpp
    ./platform/Application.cpp)
  include_directories(".")

  if (EMSCRIPTEN)
    set (CMAKE_EXECUTABLE_SUFFIX ".html")
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Oz") # optional
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file ")
  elseif (APPLE)
    find_package (OpenGL REQUIRED)
    find_library (SDL2_LIBRARIES SDL2)
    find_path (SDL2_INCLUDE_DIRS SDL.h)
    set (EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${SDL2_LIBRARIES} )
  else (EMSCRIPTEN)
    find_library (OPENGL_LIBRARY GL PATHS "/usr/lib64/nvidia" )
    find_library (SDL2_LIBRARIES SDL2 HINTS "$ENV{HOME}/local/lib")
    find_path (SDL2_INCLUDE_DIRS "SDL2/SDL.h"
      HINTS "$ENV{HOME}/local/include/"
      PATH_SUFFIXES "" "SDL2")
    include_directories(${SDL2_INCLUDE_DIRS})
    set (EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${SDL2_LIBRARIES} )
  endif (EMSCRIPTEN)
  

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

endif(TB_BUILD_DEMO_SDL2)

