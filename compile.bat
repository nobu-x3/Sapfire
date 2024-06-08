cmake -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang++.exe" -DCMAKE_C_COMPILER="C:/Program Files/LLVM/bin/clang.exe" -Wdeprecated-declarations -DROOT_PATH_ABS=%cd% -DQT_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6 -DQT_QMAKE_EXECUTABLE:FILEPATH=C:/Qt/6.5.3/msvc2019_64/bin/qmake.exe -DQt6CoreTools_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6CoreTools -DQt6Core_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6Core -DQt6EntryPointPrivate_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6EntryPointPrivate -DQt6GuiTools_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6GuiTools -DQt6Gui_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6Gui -DQt6WidgetsTools_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6WidgetsTools -DQt6Widgets_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6Widgets -DQt6ZlibPrivate_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6ZlibPrivate -DQt6_DIR:PATH=C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6
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
xcopy /S /I /H /Q /Y editor_assets bin\editor_assets
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
