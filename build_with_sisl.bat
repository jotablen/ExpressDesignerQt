@echo off
set "MSYS2_PATH=C:\msys64"
set "PATH=%MSYS2_PATH%\mingw64\bin;%MSYS2_PATH%\usr\bin"
cd /d %~dp0

rmdir /s /q build 2>nul
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DUSE_SISL=ON -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 2>&1
if errorlevel 1 goto :end
mingw32-make -j4 2>&1
if errorlevel 1 goto :end
echo.
echo === BUILD COMPLETE ===
dir src\ExpressDesigner.exe
echo.
echo Now run: deploy_app.bat
:end