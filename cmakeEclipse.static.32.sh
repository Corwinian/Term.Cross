mkdir -p _build_/Eclipse.32
cd _build_/Eclipse.32
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -G "Eclipse CDT4 - Unix Makefiles" ../..