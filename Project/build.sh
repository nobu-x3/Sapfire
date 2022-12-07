#!/usr/bin/bash

if [[ ! -d FILE ]]; then
	mkdir build
fi

cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake ..
cmake --build .
cd ..
if [[ -e compile_commands.json ]]; then
	rm -rf compile_commands.json
fi

cp build/compile_commands.json compile_commands.json
