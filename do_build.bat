@echo off
setlocal
set "PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;C:\Windows\System32;%PATH%"
cd /d e:\Projects\newVibes\ExpressDesignerQt

:: ============================================================
:: ExpressDesigner - Build Script
::   do_build.bat         -> incremental (reuses build/)
::   do_build.bat clean   -> full clean rebuild
:: ============================================================

if /i "%~1"=="clean" (
    echo === Clean build - removing build directory ===
    rmdir /s /q build 2>nul
)

:: Always reconfigure if CMakeCache.txt is missing
if not exist build\CMakeCache.txt (
    if not exist build mkdir build
    cd build
    echo === CMake Configure ===
    C:\PROGRA~1\CMake\bin\cmake.exe .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DUSE_SISL=OFF -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe 2>&1
    if %errorlevel% neq 0 (echo === CMake configure FAILED === && exit /b 1)
    echo === CMake Configure OK ===
) else (
    cd build
    echo === CMake Refresh ===
    C:\PROGRA~1\CMake\bin\cmake.exe . 2>&1
    if %errorlevel% neq 0 (echo === CMake refresh FAILED === && exit /b 1)
)

echo.
echo === CMake Build (incremental) ===
C:\PROGRA~1\CMake\bin\cmake.exe --build . -j4 2>&1
if %errorlevel% neq 0 (echo === Build FAILED === && exit /b 1)

echo.
echo === Checking binary ===
if exist src\ExpressDesigner.exe (
    dir src\ExpressDesigner.exe 2>&1
    echo === Build Complete ===
) else (
    echo === Binary not found, searching... ===
    dir /s /b ExpressDesigner.exe 2>&1
)

endlocal