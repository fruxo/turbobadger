# Use project source dir for demo output. We have to set
# the release and debug specific defines too, for MSVC.
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})

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
aux_source_directory(./platform LOCAL_SRCS)
include_directories(".")

# Add glfw project
include_directories("thirdparty/glfw/include")
add_subdirectory(thirdparty/glfw)

# Platform specific
if(CMAKE_COMPILER_IS_MINGW)
 # Avoid dll dependencies by linking statically.
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
endif(CMAKE_COMPILER_IS_MINGW)

# Append extra flags to link all object files. Some compilers discard object files
# which doesn't seem to be used even though they are (widget factories etc.)
set(WHOLE_TB_LIB TurboBadgerLib)
if(CMAKE_COMPILER_IS_GNUCXX)
 set(WHOLE_TB_LIB -Wl,--whole-archive TurboBadgerLib -Wl,--no-whole-archive)
endif(CMAKE_COMPILER_IS_GNUCXX)
if(APPLE)
 set(WHOLE_TB_LIB -all_load TurboBadgerLib)
endif(CMAKE_COMPILER_IS_GNUCXX)
if(MSVC)
 message(WARNING "Note: There are some remaining issues to fix with cmake for MSVC!")
 # The project needs "Link library dependencies" and "Use library dependency inputs"
 # set. Something which cmake isn't able to do.
 # The solution also gets spammed with all glfws disabled sample projects.
 # set(WHOLE_TB_LIB /OPT:NOREF TurboBadgerLib)
endif(MSVC)

add_executable(TurboBadgerDemo WIN32 ${LOCAL_SRCS})
target_link_libraries(TurboBadgerDemo ${WHOLE_TB_LIB} ${EXTRA_LIBS} glfw ${GLFW_LIBRARIES})
