@mkdir debug_win32 2>nul
@rem make compile_flags="-O0 -g" --directory=debug --makefile=..\Makefile %*
@make debug=on sys=_win32 --directory=debug_win32 --makefile=..\Makefile %*

