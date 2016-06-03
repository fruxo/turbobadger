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

# Add glfw project on MacOSX using 'ports'
if(APPLE)
  find_library (GLFW_LIBRARIES glfw )
  find_library (OPENGL_LIBRARY OpenGL )
  set (EXTRA_LIBS ${EXTRA_LIBS} ${GLFW_LIBRARIES} ${OPENGL_LIBRARY})
  #message("GLFW_LIBRARIES: ${GLFW_LIBRARIES}")
  find_path (GLFW_INCLUDE_DIRS GLFW/glfw3.h)
  include_directories(${GLFW_INCLUDE_DIRS})
  #message("GLFW_INCLUDE_DIRS: ${GLFW_INCLUDE_DIRS}")
endif(APPLE)

if (NOT GLFW_LIBRARIES)
  target_include_directories(TurboBadgerLib PUBLIC "thirdparty/glfw/include")
  include_directories("thirdparty/glfw/include")
  add_subdirectory(thirdparty/glfw)
endif (NOT GLFW_LIBRARIES)

set (EXTRA_LIBS ${EXTRA_LIBS} ${GLFW_LIBRARIES})

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
    ./platform/port_sdl2.cpp)
  include_directories(".")
  
  message("HI")
  if (APPLE)
    find_package (OpenGL REQUIRED)
    find_library (SDL2_LIBRARIES SDL2)
    #find_library (SDL2_IMAGE_LIBRARIES SDL2_image)
    find_path (SDL2_INCLUDE_DIRS SDL.h)
    #find_path (SDL2_IMAGE_INCLUDE_DIRS SDL_image.h)
    #link_directories (${SDL2_LIBRARY_DIRS})
  else (APPLE)
  endif (APPLE)
  
  set (EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${SDL2_LIBRARIES} )
  message ("SDL2 lib : ${SDL2_LIBRARIES}")
  message ("SDL2 incl: ${SDL2_INCLUDE_DIRS}")

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

