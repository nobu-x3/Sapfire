#!/bin/bash

mkdir build/assets

cp -r assets build/assets

for shader in assets/shaders/* ; do
	echo "$shader -> $shader.spv"
	extension="${shader##*.}"
	if [ $extension = "vert" ]; then
		$VULKAN_SDK/bin/glslc -fshader-stage=vert $shader -o build/$shader.spv
	elif [ $extension = "frag" ]; then
		 $VULKAN_SDK/bin/glslc -fshader-stage=frag $shader -o build/$shader.spv
	else
		echo "Unsupported file $shader"
	fi
	if [[ $ERRORLEVEL -ne 0 ]]; then
		echo "Error:"ERRORLEVEL && exit
	fi
done