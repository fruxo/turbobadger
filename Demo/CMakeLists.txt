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
  DESTINATION "/"
  COMPONENT demoapp)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/resources
  DESTINATION TurboBadgerDemo_
  COMPONENT demoapp
  PATTERN "*.psd" EXCLUDE
  PATTERN "*.txt"
  PATTERN "*.svg"
  PATTERN "*.png")
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/Demo/demo01
  DESTINATION TurboBadgerDemo_/Demo
  COMPONENT demoapp
  PATTERN "*.psd" EXCLUDE
  PATTERN "*.txt"
  PATTERN "*.svg"
  PATTERN "*.png")
install (
  FILES
  ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser.tb.txt
  ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser_included.tb.txt
  ${CMAKE_SOURCE_DIR}/src/tb/tests/test_tb_parser_definitions.tb.txt
  DESTINATION TurboBadgerDemo_
  COMPONENT demoapp)

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

#################################################################################
# CPack
set (CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} PARENT_SCOPE)
set (CPACK_COMPONENTS_ALL demoapp)

set (CPACK_PACKAGE_NAME "TurboBadgerDemo")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "TurboBadger UI Demo")
set (CPACK_PACKAGE_VERSION "${VERSION}${VERSION_BUILD}")
set (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_PACKAGE_URL "https://github.com/tesch1/turbobadger")
set (CPACK_PACKAGE_VENDOR "nobody")
set (CPACK_PACKAGE_CONTACT "tesch1@gmail.com")
set (CPACK_PACKAGE_RELOCATABLE TRUE)
set (CPACK_STRIP_FILES TRUE)
set (CPACK_SOURCE_STRIP_FILES TRUE)

# WIN32
if (WIN32)
  set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-install-win32")
  set (CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
  set (CPACK_NSIS_URL_INFO_ABOUT "https://github.com/tesch1/turbobadger")
endif (WIN32)

# macOS
if (APPLE AND NOT XCODE)
  set (CPACK_GENERATOR "DragNDrop")
  set (CPACK_BUNDLE_NAME "${CPACK_PACKAGE_NAME}")
endif (APPLE AND NOT XCODE)

# unix
if (UNIX AND NOT APPLE)
  set (CPACK_GENERATOR "RPM;TGZ;DEB;STGZ")
  set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)
  set (CPACK_VERBATIM_VARIABLES TRUE) # fixes @ in rpm file names
  set (CPACK_RPM_PACKAGE_RELEASE 1)
  set (CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
endif (UNIX AND NOT APPLE)

include (CPack)
