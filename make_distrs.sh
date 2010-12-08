#!/bin/bash

./cmake.static.32.sh _build_/Makefile32 distr/linux32 install
./cmake.static.64.sh _build_/Makefile64 distr/linux64 install
./cmakeMinGW.static.sh _build_/Makefile.Win32 distr/win32_MinGW install

mkdir ~/mnt/distr/Linux/TERM.CROSS
tar cz -C distr/linux32/ . >~/mnt/distr/term.cross/linux32.tar.gz
tar cz -C distr/linux64/ . >~/mnt/distr/term.cross/linux64.tar.gz


mkdir ~/mnt/distr/Windows/TERM.CROSS
( cd distr/win32_MinGW  ; zip -r ~/mnt/distr/term.cross/win32_MInGW.zip  . )