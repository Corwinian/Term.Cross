#!/bin/bash
mkdir release_lnx 2>>/dev/null
make sys=_lnx --directory=release_lnx --makefile=../Makefile $*
