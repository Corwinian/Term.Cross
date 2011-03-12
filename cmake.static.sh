mkdir -p _build_/Makefiles
cd _build_/Makefiles
cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX:PATH=~/opt/term.cross ../..
