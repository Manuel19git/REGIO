@echo off
REM Example batch with options

REM Check first argument
if "%~1" == "" goto usage

set ACTION=%~1

REM parse action
if /I "%ACTION%"=="clean" goto do_clean
if /I "%ACTION%"=="build" goto do_build
if /I "%ACTION%"=="vs-build" goto do_vs_build
if /I "%ACTION%"=="rebuild" goto do_rebuild

echo Unknown option: %ACTION%
goto usage

:do_clean
echo Cleaning build directory...
REM e.g. del /Q /F build\*
rd /S /Q build
goto end

:do_build
echo Building project...
REM e.g. cmake + build
mkdir build
cmake -S . -B .\build -DCMAKE_CXX_COMPILER=cl -G "MinGW Makefiles"
call pushd build
call mingw32-make -s 2>&1 | findstr /V /C:"Nota:"
if ERRORLEVEL 1 (
  rem The filter removed everything — maybe there were no other lines
  rem But ensure we still have correct exit code if build failed
  mingw32-make -s
  exit /B %ERRORLEVEL%
)
call .\REGIO.exe
call popd 
goto end

:do_vs_build
mkdir build
call cmake -S . -B .\build
goto end

:do_rebuild
call :do_clean
call :do_build
goto end

:usage
echo Usage: %~n0 ^(clean ^| build ^| vs_build ^| rebuild^)
echo   clean   : remove build directory
echo   build   : configure + build using MinGW + MSVC
echo   vs-build   : configure + build using visual studio (recommended)
goto end

:end
