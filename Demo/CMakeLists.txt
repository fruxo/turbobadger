SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

if(CMAKE_SYSTEM MATCHES "Windows")
 add_definitions(-DGLFW_EXPOSE_NATIVE_WIN32)
 add_definitions(-DGLFW_EXPOSE_NATIVE_WGL)
 set(EXTRA_LIBS ${EXTRA_LIBS} winmm)
endif(CMAKE_SYSTEM MATCHES "Windows")

if(CMAKE_SYSTEM MATCHES "Linux")
 # We probably need something here
endif(CMAKE_SYSTEM MATCHES "Linux")

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

# Platform specifix
if(CMAKE_COMPILER_IS_MINGW)
# Avoid dll dependencies by linking statically.
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
endif(CMAKE_COMPILER_IS_MINGW)

# Mingw discard object files containing unit tests and widget factories
# even though they may be needed, so we must enforce linking of all contents.
# TODO: Should probably apply all GCC variants!
set(WholeTurboBadgerLib TurboBadgerLib)
if(CMAKE_COMPILER_IS_MINGW)
 set(WholeTurboBadgerLib -Wl,--whole-archive TurboBadgerLib -Wl,--no-whole-archive)
endif(CMAKE_COMPILER_IS_MINGW)

add_executable(TurboBadgerDemo WIN32 ${LOCAL_SRCS})
target_link_libraries(TurboBadgerDemo ${WholeTurboBadgerLib} ${EXTRA_LIBS} glfw ${GLFW_LIBRARIES})
