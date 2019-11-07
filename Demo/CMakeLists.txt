# Use project source dir for demo output. We have to set
# the release and debug specific defines too, for MSVC.
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR})

set (SANITIZER "${SANITIZER} -O1 -fsanitize=address -fno-omit-frame-pointer")
#set (SANITIZER "${SANITIZER} -fsanitize=leak")
#set (SANITIZER "${SANITIZER} -fsanitize=memory")
#set (SANITIZER "${SANITIZER} -fsanitize=undefined")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -g ${SANITIZER}")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include (ExternalProject)

if (CMAKE_SYSTEM MATCHES "Windows" AND TB_BUILD_DEMO_GLFW)
  add_definitions (-DGLFW_EXPOSE_NATIVE_WIN32)
  add_definitions (-DGLFW_EXPOSE_NATIVE_WGL)
  set (EXTRA_LIBS ${EXTRA_LIBS} winmm)
endif (CMAKE_SYSTEM MATCHES "Windows" AND TB_BUILD_DEMO_GLFW)

# Add sources
set (LOCAL_SRCS "")
aux_source_directory (./demo01 LOCAL_SRCS)
set (LOCAL_SRCS ${LOCAL_SRCS}
  ./platform/main.cpp
  ./platform/Application.cpp)

if (TB_BUILD_DEMO_GLFW)
  set (LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/port_glfw.cpp
    ./platform/glfw_extra_linux.cpp)
endif (TB_BUILD_DEMO_GLFW)

if (TB_BUILD_DEMO_SDL2)
  set (LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/port_sdl2.cpp)
endif (TB_BUILD_DEMO_SDL2)

include_directories(".")

add_executable (TurboBadgerDemo WIN32 ${LOCAL_SRCS})
target_link_libraries (TurboBadgerDemo TurboBadgerLib ${EXTRA_LIBS})
install (TARGETS TurboBadgerDemo
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# Platform specific
if (CMAKE_COMPILER_IS_MINGW)
  # Avoid dll dependencies by linking statically.
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
endif (CMAKE_COMPILER_IS_MINGW)

if (EMSCRIPTEN)
  set (CMAKE_EXECUTABLE_SUFFIX ".html")
  # default skin resources
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file /tmp/resources@/resources")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file /tmp/demo01@Demo/demo01")
  # test files
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser.tb.txt@/")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser_included.tb.txt@/")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser_definitions.tb.txt@/")

  add_custom_command (TARGET TurboBadgerDemo
    PRE_LINK
    COMMAND rm -rf /tmp/resources /tmp/demo01
    COMMAND rsync -a --exclude='*.psd' ${CMAKE_SOURCE_DIR}/resources /tmp/
    COMMAND rsync -a --include='*/' --include='*.txt' --include='*.png' --exclude='*' ${CMAKE_CURRENT_SOURCE_DIR}/demo01 /tmp )
endif (EMSCRIPTEN)
