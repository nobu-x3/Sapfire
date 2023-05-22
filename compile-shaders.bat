@echo off
FOR %%s IN (assets\shaders\*) DO (
    echo "%%s%"
    IF "%%~xs"==".frag" (
        %VULKAN_SDK%\bin\glslc.exe -fshader-stage=frag %%s -o %%s.spv
        IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
    ) else (
        IF "%%~xs"==".vert" (
             %VULKAN_SDK%\bin\glslc.exe -fshader-stage=vert %%s -o %%s.spv
             IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
        )
    )
)
xcopy /h /i /c /k /e /r /y "assets" "bin\assets"
