@echo off

set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
    echo vswhere not found
    exit /b 1
)

for /f "usebackq delims=" %%i in (`%VSWHERE% -latest -property installationPath`) do (
    set VS_PATH=%%i
)

call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64
