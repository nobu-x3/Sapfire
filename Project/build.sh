#!/usr/bin/bash

if [[ ! -d FILE ]]; then
	mkdir build
fi

cd build
cmake -H. -BDebug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
cmake ..
cmake --build .
cd ..
if [[ -e compile_commands.json ]]; then
	rm -rf compile_commands.json
fi

cp build/compile_commands.json compile_commands.json
