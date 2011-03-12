#!/bin/bash

P=`pwd`
BUILD_DIR="${1:-_build_/win32_MinGW}"
INSTALL_DIR="${2:-~/opt/term.cross}"
if [[ "$3" = "install" ]] ; then
   mkdir -p "$INSTALL_DIR"
fi
INSTALL_DIR=`cd "$INSTALL_DIR" ; pwd`
mkdir -p "${BUILD_DIR}"
BUILD_DIR=`cd "$BUILD_DIR" ; pwd`


rm -fr "${BUILD_DIR}/CMakeCache.txt"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

P2=`winepath  -w "$P"`
BUILD_DIR2=`winepath  -w "."`
INSTALL_DIR2=`winepath  -w "$INSTALL_DIR"`

wine cmake  \
    -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_BUILD_TYPE=Debug  \
    -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_GENMETA:BOOL=OFF -G "MinGW Makefiles" \
    -DCMAKE_INSTALL_PREFIX:PATH="${INSTALL_DIR2}" "$P2" 2>&1 | tee cmake.log

if [[ "$3" = "install" ]] ; then
    wine make 2>&1 >make.log
    wine make install 2>&1 >make_install.log
    cp cmake.log make.log make_install.log  "${INSTALL_DIR}"
fi; 

# ставим MinGW и cmake под wine (http://www.mingw.org/wiki/Getting_Started)
# cd .wine/drive_c/
# wget http://sourceforge.net/downloads/mingw/Automated%20MinGW%20Installer/mingw-get/mingw-get-0.1-mingw32-alpha-2-bin.tar.gz
# tar xvfz mingw-get-0.1-mingw32-alpha-2-bin.tar.gz
# cd bin
# wine mingw-get install mingwrt w32api binutils gcc g++ mingw32-make
# cd ..
# wine regedit
# rem HKEY_CURRENT_USER/Environment/PATH <= C:\WinGW\bin
# wget http://www.cmake.org/files/v2.8/cmake-2.8.1-win32-x86.exe 
# rem http://www.cmake.org/cmake/resources/software.html
# wine cmake-2.8.1-win32-x86.exe
# wine copy c:\\MinGW\\bin\\mingw32-make.exe c:\\MinGW\\bin\\make.exe 
# wine copy "c:\\Program Files\\CMake 2.8\\share\\cmake-2.8\\Modules\\CMakeFortranInformation.cmake" "c:\\"
