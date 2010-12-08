@mkdir debug_win32 2>nul
@make debug=on sys=_win32 --directory=debug_win32 --makefile=..\Makefile %*

