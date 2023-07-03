mkdir -p log
mkdir -p build_${1}
cd build_${1}
cmake .. -DCMAKE_BUILD_TYPE=${1} -DBUILD_WITH_ASAN=true
cd ..