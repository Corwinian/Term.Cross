#!/bin/bash
mkdir debug_lnx 2>>/dev/null
make debug=on sys=_lnx --directory=debug_lnx --makefile=../Makefile $*

