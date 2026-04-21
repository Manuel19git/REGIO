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
rem Time to build on Asus V3 (~7 min)
mkdir build
call cmake -S . -B .\build -DCMAKE_BUILD_TYPE=Release
call cmake --build build --config Release
goto end

:do_rebuild
call :do_clean
call :do_build
goto end

:usage
echo Usage: %~n0 ^(clean ^| build_ninja ^| build_vs ^| rebuild^)
echo   clean   	   : remove build directory
echo   build : configure + build using Ninja + MSVC
echo   build_vs    : configure + build using visual studio
goto end

:end
