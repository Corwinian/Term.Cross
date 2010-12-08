@mkdir release_win32 2>>nul
@rem make compile_flags="-O3 -Wall -fmessage-length=0" --directory=release --makefile=..\Makefile %*
@make sys=_win32 --directory=release_win32 --makefile=../Makefile %*
