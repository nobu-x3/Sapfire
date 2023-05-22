mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.. -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE -DBUILD_SHARED_LIBS=TRUE
ninja
cd ..
xcopy /h /i /c /k /e /r /y build\sandbox\sandbox.exe bin\sandbox.exe
xcopy /h /i /c /k /e /r /y build\sapfire\sapfire.lib bin\sapfire.lib
xcopy /h /i /c /k /e /r /y build\sapfire\sapfire.dll bin\sapfire.dll
.\bin\sandbox.exe