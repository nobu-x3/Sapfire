del /q bin\Sapling.exe
del /q build\sapling\Sapling.exe
call compile.bat
xcopy /S /I /H /Q /Y test_proj.sfproj bin
xcopy /S /I /H /Q /Y test_scene.scene bin
bin\Sapling.exe
