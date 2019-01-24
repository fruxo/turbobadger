#!/bin/bash

# die on errors
set -e

: ${VERBOSE=0}
: ${BUILD_DIR=Build}
: ${MAKE_FLAGS=}
: ${CMAKE_FLAGS=-DTB_SYSTEM_LINUX=ON}

SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

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
        -o)                    BUILD_DIR=$(mkdir -p "$2" && cd "$2" && pwd); shift ;;
        -C|--clang)            export CC=clang; export CXX=clang++ ;;
        -gl)
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_RENDERER_GL=ON"
            ;;
        -gl3)
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_RENDERER_GL=ON -DTB_RENDERER_GL3=ON"
            ;;
        -gles2)
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_RENDERER_GL=ON -DTB_RENDERER_GLES_2=ON"
            ;;
        -em*)
            BUILD_DIR="BuildEmsc"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_BUILD_DEMO_SDL2=ON"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_BUILD_DEMO_GLFW=OFF"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_SYSTEM_SDL=ON"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_SYSTEM_LINUX=ON"
            source ${HOME}/local/emsdk/emsdk_env.sh
            #${EMSCRIPTEN}/emcc --clear-cache --clear-ports
            CMAKE_FLAGS="${CMAKE_FLAGS} -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN}/cmake/Modules/Platform/Emscripten.cmake"
            #CMAKE_FLAGS="${CMAKE_FLAGS} -G Unix Makefiles"
            ;;
        -sdl*)
            BUILD_DIR="BuildSDL"
            #CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_GET_SDL2=ON"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_BUILD_DEMO_SDL2=ON"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_BUILD_DEMO_GLFW=OFF"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_SYSTEM_SDL=ON"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_SYSTEM_LINUX=ON"
            ;;
        -glfw)
            BUILD_DIR="BuildGLFW"
            #CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_GET_GLFW=ON"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_BUILD_DEMO_SDL2=OFF"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_BUILD_DEMO_GLFW=ON"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_CLIPBOARD_GLFW=ON"
            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_SYSTEM_LINUX=ON"

            CMAKE_FLAGS="${CMAKE_FLAGS} -DTB_RENDERER_GL=ON"
            ;;
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

cd "${SRC_DIR}"
if [ ! -f ./integration.txt ]; then
    echo "run build.sh from turbobadger root directory"
    exit 1
fi

if [ ! -f Demo/thirdparty/glfw/CMakeLists.txt ]; then
    git submodule init
    git submodule update
fi

if [ ! -z "${BUILD_DIR}" ] && [ -d "${BUILD_DIR}" ]; then
    rm -rf "${BUILD_DIR}"
fi
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
cmake "${SRC_DIR}" ${CMAKE_FLAGS} -G 'Unix Makefiles'
make -j${NPROC} ${MAKE_FLAGS}
