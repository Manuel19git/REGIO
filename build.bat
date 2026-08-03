@echo off

REM Check first argument
if "%~1" == "" goto usage

set ACTION=%1
set CONFIG=%2

if /I "%CONFIG%" == "" set CONFIG=Debug
if /I "%CONFIG%" == "release" set CONFIG=Release

echo Action: %ACTION%
echo Config: %CONFIG%


REM parse action
if /I "%ACTION%"=="clean" goto do_clean
if /I "%ACTION%"=="build" goto do_build
if /I "%ACTION%"=="build_vs" goto do_build_vs
if /I "%ACTION%"=="rebuild" goto do_rebuild

echo Unknown option: %ACTION%
goto usage

:do_clean
echo Cleaning build directory...
REM e.g. del /Q /F build\*
rd /S /Q build
goto end

:do_build
echo Building project with Ninja...
mkdir build

rem I need to have visual studio IDE installed
if not defined VCINSTALLDIR (
    echo MSVC environment not found. Loading...
    call .\set-visual-env.bat
) else (
    echo MSVC environment already active (%VisualStudioVersion%)
)

rem mingw32 should not be in the env path for cmake to find the correct linker
call cmake -S . -B .\build -DCMAKE_CXX_COMPILER=cl -G "Ninja Multi-Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
call cmake --build .\build --config %CONFIG%
if %ERRORLEVEL% == 0 (
   echo Launching .\build\%CONFIG%\REGIO.exe
   call .\build\%CONFIG%\REGIO.exe
)
if %ERRORLEVEL% == 1 (
   exit /B1
)
goto end

:do_build_vs
:: Define the path to vswhere.exe (Standard Microsoft installation path)
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist %VSWHERE% (
    echo [ERROR] vswhere.exe not found. Visual Studio 2017 or newer is likely not installed.
    exit /b 1
)
:: Query the major version number of the latest installed VS instance
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -property installationVersion`) do (
    set VS_VERSION=%%i
)

:: Extract just the major version number (e.g., "17" from "17.4.33103.184")
for /f "delims=." %%a in ("%VS_VERSION%") do (set MAJOR_VERSION=%%a)

:: Map the major version to the correct CMake Generator string
if "%MAJOR_VERSION%"=="17" (
    set "GENERATOR=Visual Studio 17 2022"
    set "ARCH_FLAG=-A x64"
) else if "%MAJOR_VERSION%"=="16" (
    set "GENERATOR=Visual Studio 16 2019"
    set "ARCH_FLAG=-A x64"
) else if "%MAJOR_VERSION%"=="15" (
    :: VS 2017 requires the architecture appended directly to the generator string
    set "GENERATOR=Visual Studio 15 2017 Win64"
    set "ARCH_FLAG="
) else (
    echo "[ERROR] Supported Visual Studio version not found (Detected major version: %MAJOR_VERSION%)"
    exit /b 1
)

echo [INFO] Found %GENERATOR%
echo [INFO] Running CMake Configuration...

mkdir build_VS
call cmake -S . -B .\build_VS -G "%GENERATOR%" %ARCH_FLAG% -DCMAKE_BUILD_TYPE=%CONIFG%
call cmake --build build_VS --config %CONFIG%
goto end

:do_rebuild
call :do_clean
call :do_build
goto end

:usage
echo Usage: %~n0 ^(clean ^| build ^| build_vs ^) ^(debug ^| release) Note: debug config set by default
echo   clean   	   : remove build directory
echo   build       : configure + build using Ninja + MSVC
echo   build_vs    : configure + build using visual studio
goto end

:end
