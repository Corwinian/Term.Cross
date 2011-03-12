rm -rf _build_/Makefiles.wogenmeta.64
mkdir -p _build_/Makefiles.wogenmeta.64
cd _build_/Makefiles.wogenmeta.64
cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_GENMETA:BOOL=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX:PATH=~/opt/term.cross ../..
