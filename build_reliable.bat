@echo off
setlocal
set "P=C:\msys64\mingw64\bin;C:\msys64\usr\bin;C:\Windows\System32"
set "PATH=%P%;%PATH%"
set "R=%~dp0"
if "%R:~-1%"=="\" set "R=%R:~0,-1%"
rmdir /s /q "%R%\build" 2>nul
mkdir "%R%\build"
cd /d "%R%\build"
C:\PROGRA~1\CMake\bin\cmake.exe "%R%" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DUSE_SISL=ON -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe
if errorlevel 1 exit /b 1
C:\msys64\mingw64\bin\mingw32-make.exe sisl -j1
if errorlevel 1 exit /b 1
C:\msys64\mingw64\bin\mingw32-make.exe ExpressDesigner -j1
if errorlevel 1 exit /b 1
echo "BUILD OK"