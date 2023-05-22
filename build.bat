mkdir build
cd build
::cmake .. -DCMAKE_MAKE_PROGRAM="make" -G "MinGW Makefiles" -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.. -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 
::make
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.. -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE -DBUILD_SHARED_LIBS=TRUE
ninja
cd ..
xcopy /h /i /c /k /e /r /y build\sandbox\sandbox.exe bin\sandbox.exe
xcopy /h /i /c /k /e /r /y build\sapfire\sapfire.lib bin\sapfire.lib
xcopy /h /i /c /k /e /r /y build\sapfire\sapfire.dll bin\sapfire.dll
cd bin
.\sandbox.exe
@REM REM Engine
@REM make -f "Makefile.sapfire.windows.mak" all
@REM IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

@REM REM Testbed
@REM make -f "Makefile.sanbox.windows.mak" all
@REM IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)