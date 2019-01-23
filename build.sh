#!/bin/bash

# die on errors
set -e

: ${VERBOSE=0}
: ${MAKE_FLAGS=}
: ${CMAKE_FLAGS=-DTB_SYSTEM_LINUX=ON}

if command -v nproc &> /dev/null ; then
    NPROC=$(nproc)
elif command -v sysctl &> /dev/null ; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=1
fi

usage () {
    echo "usage: ./build.sh [-v|-q] [-gl3] [-sdl]"
    exit 1
}

# process command line args
while [ $# -gt 0 ]; do
    key="$1"
    case $key in
        -gl3)                  CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_RENDERER_GL3=ON" ;;
        -sdl)                  CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_BUILD_DEMO_SDL2=ON -DTB_BUILD_DEMO_GLFW=OFF" ;;
        -v|--verbose)          VERBOSE=$(( ${VERBOSE} + 1 ))
                               MAKE_FLAGS="${MAKE_FLAGS} VERBOSE=1"
                               ;;
        -q|--quiet)            VERBOSE=$(( ${VERBOSE} - 1 ))
                               ;;
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

mkdir -p Build
cd Build
cmake ../ ${CMAKE_FLAGS}
make -j${NPROC} ${MAKE_FLAGS}
