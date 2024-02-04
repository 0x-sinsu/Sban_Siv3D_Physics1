@echo off
setlocal

for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set MSBUILD_PATH=%%i\MSBuild\Current\Bin\MSBuild.exe
)

if not exist "%MSBUILD_PATH%" (
    echo Path to MSBuild not found.
    exit /b 1
)

"%MSBUILD_PATH%" SbanPhysics1.sln /p:Configuration=Release /p:Platform="x64"

endlocal
