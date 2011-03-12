mkdir -p _build_/CodeBlocks
cd _build_/CodeBlocks
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -G "CodeBlocks - Unix Makefiles" ../..
