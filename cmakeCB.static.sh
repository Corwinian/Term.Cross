mkdir -p _build_/CodeBlocks
cd _build_/CodeBlocks
cmake -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS:BOOL=OFF -G "CodeBlocks - Unix Makefiles" ../..
