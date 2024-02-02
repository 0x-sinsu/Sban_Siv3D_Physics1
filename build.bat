@echo off
setlocal

:: MSBuild のパスを取得する（vswhere を使用）
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set MSBUILD_PATH=%%i\MSBuild\Current\Bin\MSBuild.exe
)

:: MSBuild のパスが見つかったかチェック
if not exist "%MSBUILD_PATH%" (
    echo MSBuild のパスが見つかりませんでした。
    exit /b 1
)

:: MSBuild でビルドを実行
"%MSBUILD_PATH%" SbanPhysics1.sln /p:Configuration=Release /p:Platform="x64"

:: 終了
endlocal
