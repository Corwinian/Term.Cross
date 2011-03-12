rm -rf _build_/Makefiles.MSYS
mkdir -p _build_/Makefiles.MSYS
cd _build_/Makefiles.MSYS
#cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX:PATH=/opt/term.cross ../..
cmake -G "MSYS Makefiles" -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/opt/term.cross ../..
