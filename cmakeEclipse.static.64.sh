mkdir -p _build_/Eclipse.64
cd _build_/Eclipse.64
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS:BOOL=OFF -G "Eclipse CDT4 - Unix Makefiles" ../..
