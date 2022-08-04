mkdir -p build_sandbox
cd build_sandbox
cmake -DCMAKE_BUILD_TYPE=Debug ../sandbox/
make -j 8
mv vulture-sandbox ../
cd ..
