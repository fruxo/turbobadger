#!/bin/bash

# die on errors
set -e

: ${VERBOSE=0}

usage () {
    echo "usage: ./build.sh [-v]"
    exit 1
}

# process command line args
while [ $# -gt 0 ]; do
    key="$1"
    case $key in
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
cmake ../

if command -v nproc &> /dev/null ; then
    NPROC=$(nproc)
elif command -v sysctl &> /dev/null ; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=1
fi

make -j${NPROC} VERBOSE=${VERBOSE}
