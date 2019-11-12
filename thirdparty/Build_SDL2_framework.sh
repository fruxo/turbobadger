#!/bin/sh
#
# got parts of this from
# https://github.com/mthiesen/SDL2-iOS-Prebuilt-Framework/blob/master/Scripts/build-framework.sh
#
#set -x
set -e

TYPE=$1
TMP_BUILD_DIR=$(mktemp -d -t "SDL-$TYPE-XXX")
#TMP_BUILD_DIR=/tmp/blah

SDL_VERSION="2.0.10"
SDL_SRCDIR="../SDL2-${SDL_VERSION}"
SDL_URL="https://www.libsdl.org/release/"
SDL_FILENAME="SDL2-${SDL_VERSION}.tar.gz"
WGET_CURL=wget
CMDLINE="$0 $@"
SCRIPT=$(basename "$0")
SRCDIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OnError() {
    echo "$SCRIPT: Error on line ${BASH_LINENO[0]}, exiting."
    exit 1
}
trap OnError ERR

cd "$SRCDIR/.."
if ! [ -f src/tb/tb_core.h ]; then
    echo "Run $0 from the turbobadger root source directory!"
    exit -1
fi

if ! [ -f "${SDL_SRCDIR}/include/SDL.h" ] ; then
    if command -v wget >/dev/null 2>&1 ; then
        WGET_CURL=wget
    elif command -v curl >/dev/null 2>&1 ; then
        WGET_CURL="curl -O"
    else
        echo "need wget or curl to download SDL2"
        exit 1
    fi

    ${WGET_CURL} ${SDL_URL}/${SDL_FILENAME}
    tar xzvf "${SDL_FILENAME}" -C $(dirname $SDL_SRCDIR)
fi

if ! [ -f "${SDL_SRCDIR}/include/SDL.h" ] ; then
    echo "SDL_SRCDIR not set right."
    exit -1
fi

function cleanup {
    #    rm -rf "$TMP_BUILD_DIR"
    echo
}
trap cleanup EXIT

function make_framework {
    local BASEDIR="$1"
    local FRAMEWORK="$2"
    local VERSION="$3"
    local HEADER_DIR="$4"
    mkdir -p "$BASEDIR/${FRAMEWORK}.framework/Versions/${VERSION}/Headers"
    cp "$BASEDIR/lib${FRAMEWORK}.a" "$BASEDIR/${FRAMEWORK}.framework/Versions/${VERSION}/${FRAMEWORK}"
    cp "${HEADER_DIR}"/*.h "$BASEDIR/${FRAMEWORK}.framework/Versions/${VERSION}/Headers"
    ln -s "$VERSION" "$BASEDIR/${FRAMEWORK}.framework/Versions/Current"
    ln -s "Versions/Current/Headers" "$BASEDIR/${FRAMEWORK}.framework/Headers"
    ln -s "Versions/Current/$FRAMEWORK" "$BASEDIR/${FRAMEWORK}.framework/$FRAMEWORK"
}

if [[ "$TYPE" == ios ]] ; then
    OUTPUT_DIR="Frameworks.iOS"
    SRC_PROJECT="${SDL_SRCDIR}/Xcode-iOS/SDL/SDL.xcodeproj"
    TMP_FRAMEWORK_DIR="$TMP_BUILD_DIR/SDL2.framework"
    TARGET=libSDL-iOS # pre-2.0.10 is just libSDL

    # Build fat library
    xcrun xcodebuild -quiet OTHER_CFLAGS="-fembed-bitcode" -project "$SRC_PROJECT" -target $TARGET \
          ONLY_ACTIVE_ARCH=NO -sdk iphoneos \
          -configuration Release clean build CONFIGURATION_BUILD_DIR="$TMP_BUILD_DIR/iphoneos"
    xcrun xcodebuild -quiet OTHER_CFLAGS="-fembed-bitcode" -project "$SRC_PROJECT" -target $TARGET \
          ONLY_ACTIVE_ARCH=NO -sdk iphonesimulator \
          -configuration Release clean build CONFIGURATION_BUILD_DIR="$TMP_BUILD_DIR/iphonesimulator"

    # Fatten them up...
    xcrun lipo -create "$TMP_BUILD_DIR/iphoneos/libSDL2.a" "$TMP_BUILD_DIR/iphonesimulator/libSDL2.a" \
          -output "$TMP_BUILD_DIR/libSDL2.a"

    # Create & populate the framework
    make_framework "$TMP_BUILD_DIR" SDL2 A "${SDL_SRCDIR}/include"

    if [ -d $TMP_FRAMEWORK_DIR ]; then
        # Copy the framework to its final target location
        rm -rf $OUTPUT_DIR/SDL2.framework
        mkdir -p $OUTPUT_DIR
        cp -R $TMP_FRAMEWORK_DIR $OUTPUT_DIR
    fi

    #
    # GBDeviceInfo
    #
    if false; then
        GB_SRCDIR=GBDeviceInfo
        SRC_PROJECT="${GB_SRCDIR}/GBDeviceInfo.xcodeproj"
        GTMP_FRAMEWORK_DIR="$TMP_BUILD_DIR/iphoneos/GBDeviceInfo.framework"
        xcrun xcodebuild -quiet OTHER_CFLAGS="-fembed-bitcode" -project "$SRC_PROJECT" -target GBDeviceInfo-iOS \
              ONLY_ACTIVE_ARCH=NO  \
              -configuration Release clean build CONFIGURATION_BUILD_DIR="$TMP_BUILD_DIR/iphoneos"
        
        make_framework "$TMP_BUILD_DIR/iphoneos" GBDeviceInfo A "${GB_SRCDIR}/GBDeviceInfo"
        
        rm -rf $OUTPUT_DIR/GBDeviceInfo.framework
        mkdir -p $OUTPUT_DIR
        cp -R $GTMP_FRAMEWORK_DIR $OUTPUT_DIR
    fi

elif [[ "$TYPE" == osx ]] ; then
    OUTPUT_DIR="Frameworks.macOS"
    SRC_PROJECT="${SDL_SRCDIR}/Xcode/SDL/SDL.xcodeproj"
    TMP_FRAMEWORK_DIR="$TMP_BUILD_DIR/macosx10/SDL2.framework"

    # Build fat library
    #xcrun xcodebuild -project "$SRC_PROJECT" -list
    #exit -1
    # -sdk macosx10.12
    xcrun xcodebuild -quiet -project "$SRC_PROJECT" -target Framework ONLY_ACTIVE_ARCH=NO  \
          -configuration Release clean build CONFIGURATION_BUILD_DIR="$TMP_BUILD_DIR/macosx10"

    if [ -d $TMP_FRAMEWORK_DIR ]; then
        # Copy the framework to its final target location
        rm -rf $OUTPUT_DIR/SDL2.framework
        mkdir -p $OUTPUT_DIR
        cp -R $TMP_FRAMEWORK_DIR $OUTPUT_DIR
    fi

    #
    # GBDeviceInfo
    #
    if false; then
        GB_SRCDIR=GBDeviceInfo
        SRC_PROJECT="${GB_SRCDIR}/GBDeviceInfo.xcodeproj"
        GTMP_FRAMEWORK_DIR="$TMP_BUILD_DIR/macosx10/GBDeviceInfo.framework"
        xcrun xcodebuild -quiet -project "$SRC_PROJECT" -target GBDeviceInfo-OSX ONLY_ACTIVE_ARCH=NO  \
              -configuration Release clean build CONFIGURATION_BUILD_DIR="$TMP_BUILD_DIR/macosx10"
        
        rm -rf $OUTPUT_DIR/GBDeviceInfo.framework
        mkdir -p $OUTPUT_DIR
        cp -R $GTMP_FRAMEWORK_DIR $OUTPUT_DIR
        cp ${GB_SRCDIR}/GBDeviceInfo/*.h $OUTPUT_DIR/GBDeviceInfo.framework/Headers/
    fi
    
elif [[ "$TYPE" == cc ]] ; then
    OUTPUT_DIR="${HOME}/local"
    SRC_PROJECT="${SDL_SRCDIR}/"
    TMP_FRAMEWORK_DIR="$TMP_BUILD_DIR/noexist"
    cd "$SRC_DIR"
    
else
    echo "usage: $0 <ios|osx|cc>"
    exit -1
fi

