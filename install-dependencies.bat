mkdir deps
cd deps
git clone https://github.com/libsdl-org/SDL.git
cd SDL
git checkout release-2.26.5
mkdir build
cd build
cmake .. -DCMAKE_MAKE_PROGRAM="make" -G "MinGW Makefiles" -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON 
make -j
make install