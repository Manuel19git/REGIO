@echo off

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
rem Time to build on Asus V3 (~8 min)
echo Building project...
set "MAKE_ERROR=0"

mkdir build
rem TODO: generate makefiles with ninja (mingw and windows doesn't seem to get along)
cmake -S . -B .\build -DCMAKE_CXX_COMPILER=cl -G "MinGW Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
call pushd build
(mingw32-make -s || echo !errorlevel! > error.tag) | findstr /V /C:"Nota:"
rem call mingw32-make -s > build_output.tmp 2>&1
rem set MAKE_ERROR=%ERRORLEVEL%
rem type build_output.tmp | findstr /V /C:"Nota:"
if not exist error.tag (
  call .\REGIO.exe
)
if exist error.tag (
  rem The filter removed everything — maybe there were no other lines
  rem But ensure we still have correct exit code if build failed
  del error.tag
  call popd
  exit /B 1
)

call popd 
goto end

:do_vs_build
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
echo Usage: %~n0 ^(clean ^| build ^| vs_build ^| rebuild^)
echo   clean   : remove build directory
echo   build   : configure + build using MinGW + MSVC
echo   vs-build   : configure + build using visual studio (recommended)
goto end

:end
