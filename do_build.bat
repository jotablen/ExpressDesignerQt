@echo off
setlocal
set "PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;C:\Windows\System32;%PATH%"
cd /d e:\Projects\newVibes\ExpressDesignerQt

:: ============================================================
:: ExpressDesigner - Build Script (Incremental)
:: ============================================================
:: First run: full clean configure + build
:: Subsequent runs: incremental build (only changed files)
:: To force a clean rebuild, pass "clean" as argument
:: ============================================================

if /i "%~1"=="clean" (
    echo === Clean build requested - removing build directory ===
    rmdir /s /q build 2>nul
)

if not exist build (
    mkdir build
    cd build
    echo === CMake Configure (first time) ===
    C:\PROGRA~1\CMake\bin\cmake.exe .. -G "MinGW Makefiles" ^
        -DCMAKE_BUILD_TYPE=Debug ^
        -DBUILD_TESTS=OFF ^
        -DUSE_SISL=OFF ^
        -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 ^
        -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe ^
        -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe 2>&1
    if %errorlevel% neq 0 (
        echo === CMake configure FAILED ===
        exit /b 1
    )
    echo === CMake Configure OK ===
) else (
    cd build
    :: Check if CMake needs re-run (new files added)
    if exist CMakeCache.txt (
        C:\PROGRA~1\CMake\bin\cmake.exe . 2>&1
    )
)

echo.
echo === CMake Build (incremental) ===
C:\PROGRA~1\CMake\bin\cmake.exe --build . -j4 2>&1
if %errorlevel% neq 0 (
    echo === Build FAILED ===
    exit /b 1
)

echo.
echo === Checking binary ===
if exist src\ExpressDesigner.exe (
    dir src\ExpressDesigner.exe 2>&1
    echo === Build Complete ===
) else (
    echo === Binary not found in expected location ===
    echo Checking for ExpressDesigner.exe...
    dir /s /b ExpressDesigner.exe 2>&1
)

endlocal