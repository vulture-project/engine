for shader in assets/.vulture/shaders/*.vert
do
  binary_file="${shader}.spv"
  
  echo "glslc ${shader} -o ${binary_file}"
  glslc ${shader} -o ${binary_file}
done

for shader in assets/.vulture/shaders/*.frag
do
  binary_file="${shader}.spv"
  
  echo "glslc ${shader} -o ${binary_file}"
  glslc ${shader} -o ${binary_file}
done

for shader in assets/.vulture/shaders/*.geom
do
  binary_file="${shader}.spv"
  
  echo "glslc ${shader} -o ${binary_file}"
  glslc ${shader} -o ${binary_file}
done