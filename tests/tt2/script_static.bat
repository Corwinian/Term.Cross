@rem Static library demo

@rem Create static library's object file, libhello-static.o.
@rem I'm using the name libhello-static to clearly
@rem differentiate the static library from the
@rem dynamic library examples, but you don't need to use
@rem "-static" in the names of your
@rem object files or static libraries.

@rem g++ -Wall -g -c -o libhello-static.o libhello.cpp

@rem Create static library.

@rem ar rcs libhello-static.a libhello-static.o

@rem At this point we could just copy libhello-static.a
@rem somewhere else to use it.
@rem For demo purposes, we'll just keep the library
@rem in the current directory.

@rem Compile demo_use program file.

g++ -Wall -g -c -I../include -I../c_lib/include demo_use.cpp -o demo_use.o

@rem Create demo_use program; -L. causes "." to be searched during
@rem creation of the program.  Note that this command causes
@rem the relevant object file in libhello-static.a to be
@rem incorporated into file demo_use_static.

@rem g++ -Wall -g -c -I../include -I../c_lib/include demo_use.cpp -o demo_use.o
g++ demo_use.o -static -lc_lib -L. -o demo_use_static.exe

@rem Execute the program.

rem demo_use_static
