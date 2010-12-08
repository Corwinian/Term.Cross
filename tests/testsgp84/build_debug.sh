#!/bin/bash
mkdir debug_lnx 2>/dev/null
#@rem make compile_flags="-O0 -g" --directory=debug --makefile=..\Makefile %*
make instdir=$TERM_INST debug=on sys=_lnx --directory=debug_lnx --makefile=../Makefile $*

