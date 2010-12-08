g++ -Wall -g -c -I../include -I../c_lib/include demo_use.cpp -o demo_use.o
g++ demo_use.o -lc_lib -L. -LC:/emx/lib -o demo_use_static.exe
