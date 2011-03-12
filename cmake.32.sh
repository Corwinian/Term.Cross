mkdir -p _build_/Makefiles.32
cd _build_/Makefiles.32
cmake -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_INSTALL_PREFIX:PATH=~/opt/term.cross ../..
