mkdir -p build
cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=${1} -DBUILD_WITH_ASAN=true
ninja
cd ..