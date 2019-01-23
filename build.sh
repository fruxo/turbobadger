#!/bin/bash

# die on errors
set -e

: ${VERBOSE=0}
: ${CMAKE_FLAGS=}

usage () {
    echo "usage: ./build.sh [-v]"
    exit 1
}

# process command line args
while [ $# -gt 0 ]; do
    key="$1"
    case $key in
        -gl3)                  CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_RENDERER_GL3=ON" ;;
        -sdl)                  CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_BUILD_DEMO_SDL2=ON -DTB_BUILD_DEMO_GLFW=OFF" ;;
        -v|--verbose)          VERBOSE=$(( ${VERBOSE} + 1 )) ;;
        -q|--quiet)            VERBOSE=$(( ${VERBOSE} - 1 )) ;;
        *)
            # unknown option
            echo "unknown option $key"
            usage
            ;;
    esac
    shift
done

if [ ! -f ./integration.txt ]; then
    echo "run build.sh from turbobadger root directory"
    exit 1
fi

if [ ! -f Demo/thirdparty/glfw/CMakeLists.txt ]; then
    git submodule init
    git submodule update
fi

mkdir -p build
cd build
cmake ../ ${CMAKE_FLAGS}

if command -v nproc &> /dev/null ; then
    NPROC=$(nproc)
elif command -v sysctl &> /dev/null ; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=1
fi

make -j${NPROC} VERBOSE=${VERBOSE}
