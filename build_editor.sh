mkdir -p build_editor
cd build_editor
cmake -DCMAKE_BUILD_TYPE=Debug ../editor/
make -j 8
mv vulture-editor ../
cd ..
