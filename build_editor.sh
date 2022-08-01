mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../editor/
make -j 8
mv vulture-editor ../
cd ..
