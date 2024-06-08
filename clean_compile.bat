call clean.bat
mkdir build
mkdir bin
xcopy /S /I /H /Q /Y assets build\assets
xcopy /S /I /H /Q /Y engine\vendor\dxc\lib\x64\dxcompiler.lib build\engine\vendor\dxc\lib\x64\
xcopy /S /I /H /Q /Y engine\vendor\dxc\lib\x64\dxcompiler.pdb build\engine\vendor\dxc\lib\x64\
xcopy /S /I /H /Q /Y engine\vendor\dxc\bin\x64\dxil.dll build\engine\vendor\dxc\lib\x64\
xcopy /S /I /H /Q /Y engine\vendor\dxc\bin\x64\dxil.pdb build\engine\vendor\dxc\lib\x64\
xcopy /S /I /H /Q /Y engine\vendor\DirectXAgilitySDK\bin\x64\*.pdb build
xcopy /S /I /H /Q /Y engine\vendor\DirectXAgilitySDK\bin\x64\*.dll build
xcopy /S /I /H /Q /Y engine\vendor\DirectXTK12\Bin\Windows10_2022\x64\Release\*.pdb build\engine\vendor\DirectXTK12\Bin\Windows10_2022\x64\Release\
xcopy /S /I /H /Q /Y engine\vendor\DirectXTK12\Bin\Windows10_2022\x64\Release\*.lib build\engine\vendor\DirectXTK12\Bin\Windows10_2022\x64\Release\
del /q bin\*
call compile.bat
