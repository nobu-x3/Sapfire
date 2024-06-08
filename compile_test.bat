cmake -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang++.exe" -DCMAKE_C_COMPILER="C:/Program Files/LLVM/bin/clang.exe" -Wdeprecated-declarations -DROOT_PATH_ABS=%cd% -DBUILD_TESTS=True
cd build
:: ninja Shaders
ninja
cd ..
xcopy /S /I /H /Q /Y build\*.dll bin
xcopy /S /I /H /Q /Y build\*.lib bin
xcopy /S /I /H /Q /Y build\*.exe bin
xcopy /S /I /H /Q /Y build\engine\vendor\*.dll bin
xcopy /S /I /H /Q /Y build\engine\vendor\*.lib bin
xcopy /S /I /H /Q /Y build\engine\vendor\*.exe bin
xcopy /S /I /H /Q /Y build\assets bin\assets
::xcopy /S /I /H /Q /Y build\vendor\dxc\lib\x64\dxcompiler.lib bin
::xcopy /S /I /H /Q /Y vendor\dxc\lib\x64\dxcompiler.pdb bin
xcopy /S /I /H /Q /Y engine\vendor\dxc\bin\x64\dxil.dll bin
xcopy /S /I /H /Q /Y engine\vendor\dxc\bin\x64\dxil.pdb bin
:: xcopy /S /I /H /Q /Y build\vendor\DirectXTK12\Bin\Windows10_2022\x64\Release\* bin
:: xcopy /S /I /H /Q /Y SaplingQt\vendor\Qt6\lib\*.dll bin
:: xcopy /S /I /H /Q /Y SaplingQt\vendor\Qt6\lib\*.lib bin
:: xcopy /S /I /H /Q /Y SaplingQt\vendor\Qt6\lib\*.pdb bin
move bin\engine\vendor\spdlog\* bin
move bin\sapling\* bin
xcopy /S /I /H /Q /Y build\tests\Tests.exe bin
