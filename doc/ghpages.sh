#!/bin/bash
#
# Generage the gh-pages branch from this doxygen
#
set -xe

SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. && pwd )"

# make sure we're in the right place
cd "${SRC_DIR}"
[ -d DemoAndroid2/app ] || exit 1
[ -f doc/Doxyfile.in ] || exit 1

# make the docs
if [ ! -f "${SRC_DIR}/BuildDocs/doc/Doxyfile" ] ; then
    rm -rf "${SRC_DIR}/BuildDocs"
    mkdir "${SRC_DIR}/BuildDocs"
fi
cd "${SRC_DIR}/BuildDocs"
cmake ../

# get a new tb
if [ ! -f "${SRC_DIR}/BuildDocs/doc/html/index.html" ]; then
    # cleanup old doc location
    rm -rf   "${SRC_DIR}/BuildDocs/doc/html"
    mkdir -p "${SRC_DIR}/BuildDocs/doc/html"
    cd       "${SRC_DIR}/BuildDocs/doc/html"
    git clone --depth 1 --branch gh-pages git@github.com:tesch1/turbobadger.git .
fi

cd       "${SRC_DIR}/BuildDocs/doc/html"
git rm -r *

# make the docs
cd       "${SRC_DIR}/BuildDocs"
make docs
[ -f doc/html/index.html ] || exit 1

# make the demo
if [ -f "${SRC_DIR}/TurboBadgerDemoSDL.html" ] ; then
    cp "${SRC_DIR}/TurboBadgerDemoSDL.html" "${SRC_DIR}/BuildDocs/doc/html/"
    cp "${SRC_DIR}/TurboBadgerDemoSDL.wasm" "${SRC_DIR}/BuildDocs/doc/html/"
    cp "${SRC_DIR}/TurboBadgerDemoSDL.data" "${SRC_DIR}/BuildDocs/doc/html/"
    cp "${SRC_DIR}/TurboBadgerDemoSDL.js"   "${SRC_DIR}/BuildDocs/doc/html/"
fi

# check the docs in
cd       "${SRC_DIR}/BuildDocs/doc/html"
git add .
git commit . -m 'updated doxygen docs' --amend

set +x
echo "to commit to github:"
echo "cd ${SRC_DIR}/BuildDocs/doc/html && git push -f --set-upstream origin gh-pages"
echo "or to view:"
echo "xdg-open ${SRC_DIR}/BuildDocs/doc/html/index.html"
# git push -f --set-upstream origin gh-pages
