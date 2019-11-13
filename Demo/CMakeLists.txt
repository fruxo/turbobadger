# Use project source dir for demo output. We have to set
# the release and debug specific defines too, for MSVC.
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR})

#set (SANITIZER "${SANITIZER} -O1 -fsanitize=address -fno-omit-frame-pointer")
#set (SANITIZER "${SANITIZER} -fsanitize=leak")
#set (SANITIZER "${SANITIZER} -fsanitize=memory")
#set (SANITIZER "${SANITIZER} -fsanitize=undefined")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -g ${SANITIZER}")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include (ExternalProject)

if (CMAKE_SYSTEM MATCHES "Windows" AND TB_BUILD_DEMO STREQUAL GLFW)
  add_definitions (-DGLFW_EXPOSE_NATIVE_WIN32)
  add_definitions (-DGLFW_EXPOSE_NATIVE_WGL)
  set (EXTRA_LIBS ${EXTRA_LIBS} winmm)
endif (CMAKE_SYSTEM MATCHES "Windows" AND TB_BUILD_DEMO STREQUAL GLFW)

# Add sources
set (LOCAL_SRCS "")
aux_source_directory (./demo01 LOCAL_SRCS)
set (LOCAL_SRCS ${LOCAL_SRCS}
  ./platform/main.cpp
  ./platform/Application.cpp)

if (TB_BUILD_DEMO STREQUAL GLFW)
  set (LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/port_glfw.cpp
    ./platform/glfw_extra_linux.cpp)
endif (TB_BUILD_DEMO STREQUAL GLFW)

if (TB_BUILD_DEMO STREQUAL SDL2)
  set (LOCAL_SRCS ${LOCAL_SRCS}
    ./platform/port_sdl2.cpp)
endif (TB_BUILD_DEMO STREQUAL SDL2)

add_resources (RES_FILES "${CMAKE_SOURCE_DIR}/resources" "*.txt" "*.png" "*.svg")
add_resources (RES_FILES "${CMAKE_CURRENT_SOURCE_DIR}" "demo01/*.txt" "demo01/*.png" "demo01/*.svg")
add_resources (RES_FILES "${CMAKE_SOURCE_DIR}/src/tb/tests"
  test_tb_parser.tb.txt
  test_tb_parser_included.tb.txt
  test_tb_parser_definitions.tb.txt)

add_executable (TurboBadgerDemo MACOSX_BUNDLE WIN32 ${LOCAL_SRCS} ${RES_FILES})
target_include_directories (TurboBadgerDemo PRIVATE ".")
target_link_libraries (TurboBadgerDemo TurboBadgerLib ${EXTRA_LIBS})

# Individually set the file's path properties
fixup_resources (RES_FILES)

if (APPLE)
  set (APPLE_DEVTEAM_ID "$ENV{APPLE_DEVTEAM_ID}" CACHE STRING "Apple code signing team ID")
  message (STATUS "Xcode development team: '$ENV{APPLE_DEVTEAM_ID}' (Cached: ${APPLE_DEVTEAM_ID})")
  if (IOS)
    set_target_properties (TurboBadgerDemo PROPERTIES
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${APPLE_DEVTEAM_ID}"
      XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
      XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
      MACOSX_BUNDLE_GUI_IDENTIFIER "org.hastybadger.demo"
      MACOSX_BUNDLE_ICON_NAME "AppIcon"
      )
  else ()
    set_target_properties (TurboBadgerDemo PROPERTIES
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
      XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
      MACOSX_BUNDLE_GUI_IDENTIFIER "org.hastybadger.demo"
      MACOSX_BUNDLE_ICON_FILE "hastybadgerdemo.icns"
      )
    if (NOT ${APPLE_DEVTEAM_ID} STREQUAL "")
      set_target_properties (TurboBadgerDemo PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Mac Developer"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${APPLE_DEVTEAM_ID}")
    endif ()
  endif (IOS)
endif (APPLE)

install (TARGETS TurboBadgerDemo
  COMPONENT demoapp
  BUNDLE DESTINATION "./" 
  RUNTIME DESTINATION "./")

# Platform specific
if (CMAKE_COMPILER_IS_MINGW)
  # Avoid dll dependencies by linking statically.
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
endif (CMAKE_COMPILER_IS_MINGW)

if (EMSCRIPTEN)
  set (CMAKE_EXECUTABLE_SUFFIX ".html")
  # default skin resources
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file /tmp/resources@/resources")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file /tmp/demo01@demo01")
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

set (CPACK_PACKAGE_NAME "TurboBadgerDemo${TB_BUILD_DEMO}")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "TurboBadger UI Demo (${TB_BUILD_DEMO})")
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
  set (CPACK_GENERATOR "DragNDrop;TGZ")
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
