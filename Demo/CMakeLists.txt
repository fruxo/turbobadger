# Use project source dir for demo output. We have to set
# the release and debug specific defines too, for MSVC.
if(TB_DEMO_IN_SRC)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
endif()

set (SANITIZER "${SANITIZER} -O1 -fsanitize=address -fno-omit-frame-pointer")
#set (SANITIZER "${SANITIZER} -fsanitize=leak")
#set (SANITIZER "${SANITIZER} -fsanitize=memory")
set (SANITIZER "${SANITIZER} -fsanitize=undefined")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -g ${SANITIZER}")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include (ExternalProject)

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

if(TB_BUILD_DEMO_SDL2)
  # Add sources
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
    set (EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${SDL2_LIBRARIES} )
    include_directories (${SDL2_INCLUDE_DIRS})
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

  if (EMSCRIPTEN)
    add_custom_command (TARGET TurboBadgerDemoSDL
      PRE_LINK
      COMMAND rm -rf /tmp/resources /tmp/demo01
      COMMAND rsync -a --exclude='*.psd' ${CMAKE_SOURCE_DIR}/resources /tmp/
      COMMAND rsync -a --include='*/' --include='*.txt' --include='*.png' --exclude='*' ${CMAKE_CURRENT_SOURCE_DIR}/demo01 /tmp )
  endif (EMSCRIPTEN)

endif (TB_BUILD_DEMO_SDL2)

