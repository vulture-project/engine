mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../sandbox/
make -j 8
mv vulture-sandbox ../
cd ..
