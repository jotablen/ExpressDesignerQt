@echo off
set "PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;C:\Windows\System32;%PATH%"
cd /d e:\Projects\newVibes\ExpressDesignerQt
rmdir /s /q build 2>nul
mkdir build
cd build
echo === CMake Configure ===
C:\PROGRA~1\CMake\bin\cmake.exe .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DUSE_SISL=OFF -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 2>&1
echo === CMake Build ===
C:\msys64\mingw64\bin\mingw32-make.exe -j4 2>&1
echo === Checking binary ===
dir /b ExpressDesigner.exe 2>&1
echo === Build Complete ===