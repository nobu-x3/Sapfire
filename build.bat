mkdir build
rm bin/*
cd sapfire/deps/SDL2
mkdir build
cd build
cmake .. -DCMAKE_MAKE_PROGRAM="make" -G "MinGW Makefiles" -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.. -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 
make