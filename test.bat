del /q bin\Tests.exe
del /q build\tests\Tests.exe
call compile_test.bat
bin\Tests.exe
