@mkdir release_win32 2>nul
@make sys=_win32 --directory=release_win32 --makefile=..\Makefile %*
