#!/bin/bash
set echo on

rm -rf bin/*
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.. -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make
cd ..
./compile_shaders.sh
cp build/sandbox/sandbox bin/sandbox
cp build/sapfire/libsapfire.so bin/libsapfire.so
cp -r build/assets bin/assets
cd bin
./sandbox