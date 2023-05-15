#!/bin/bash

mkdir build/assets
mkdir build/assets/shaders

for shader in shaders/* ; do
	echo "$shader -> $shader.spv"
	extension="${shader##*.}"
	if [ $extension = "vert" ]; then
		$VULKAN_SDK/bin/glslc -fshader-stage=vert $shader -o build/assets/shaders/$shader.$extension.spv
	elif [ $extension = "frag" ]; then
		 $VULKAN_SDK/bin/glslc -fshader-stage=frag $shader -o build/assets/shaders/$shader.$extension.spv
	else
		echo "Unsupported file $shader"
	fi
	if [[ $ERRORLEVEL -ne 0 ]]; then
		echo "Error:"ERRORLEVEL && exit
	fi
done