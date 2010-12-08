#!/bin/bash
mkdir release_lnx 2>/dev/null
#@rem make compile_flags="-O3 -Wall -fmessage-length=0" --directory=release --makefile=..\Makefile %*
make instdir=$TERM_INST sys=_lnx --directory=release_lnx --makefile=../Makefile $*
