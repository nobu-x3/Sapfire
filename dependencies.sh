#!/bin/sh

git submodule update --init --recursive
cd sapfire/vendor/SDL2
git checkout release-2.30.6-premake
cd ../
