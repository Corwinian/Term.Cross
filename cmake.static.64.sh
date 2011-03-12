#!/bin/bash

P=`pwd`
BUILD_DIR="${1:-_build_/Makefiles.64}"
INSTALL_DIR="${2:-~/opt/term.cross}"
if [[ "$3" = "install" ]] ; then
mkdir -p "${INSTALL_DIR}"
INSTALL_DIR2=`cd "${INSTALL_DIR}" ; pwd`
fi;

rm -fr "${BUILD_DIR}/CMakeCache.txt"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake -DBUILD_SHARED_LIBS:BOOL=OFF   \
-DCMAKE_INSTALL_PREFIX:PATH="${INSTALL_DIR2}" "$P" 2>&1 | tee cmake.log

if [[ "$3" = "install" ]] ; then
make 2>&1 >make.log
make install 2>&1 >make_install.log
cp cmake.log make.log make_install.log  "${INSTALL_DIR2}"
fi;
