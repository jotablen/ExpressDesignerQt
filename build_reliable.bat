@echo off
setlocal enabledelayedexpansion

set "NOCLEAN="
if /i "%~1"=="noclean" set "NOCLEAN=1"

set "MINGW_BIN=C:\msys64\mingw64\bin"
set "MINGW_USR=C:\msys64\usr\bin"
set "PATH=%MINGW_BIN%;%MINGW_USR%;C:\Windows\System32;%PATH%"
set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
set "BUILD_DIR=%ROOT%\build"
set "CMAKE=C:\PROGRA~1\CMake\bin\cmake.exe"
set "MAKE=%MINGW_BIN%\mingw32-make.exe"

echo ============================================================
echo   ExpressDesigner - Reliable Build
echo ============================================================

if defined NOCLEAN (
    echo [Mode] Incremental ^(noclean^)
    if not exist "%BUILD_DIR%\CMakeCache.txt" (
        echo [WARN] build/CMakeCache.txt not found - full configure needed
        set "NOCLEAN="
    )
)

if not defined NOCLEAN (
    echo [Mode] Full rebuild ^(clean build directory^)
    echo [1/3] Preparing build directory...
    rmdir /s /q "%BUILD_DIR%" 2>nul
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

if not defined NOCLEAN (
    echo.
    echo [2/3] Configuring CMake ^(SISL=ON^)...
    "%CMAKE%" "%ROOT%" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DUSE_SISL=ON -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 -DCMAKE_C_COMPILER=%MINGW_BIN%/gcc.exe -DCMAKE_CXX_COMPILER=%MINGW_BIN%/g++.exe -DCMAKE_MAKE_PROGRAM=%MAKE%
    if errorlevel 1 (
        echo [ERROR] CMake configure failed
        exit /b 1
    )
    echo [OK] CMake configured.
) else (
    echo [2/3] Using existing CMake configuration...
    echo [OK] Build directory preserved.
)

echo.
echo [3/3] Building SISL library...
"%MAKE%" sisl -j1
if errorlevel 1 (
    echo [ERROR] SISL build failed
    exit /b 1
)

echo.
echo [3/3] Building ExpressDesigner...
"%MAKE%" ExpressDesigner -j1
if errorlevel 1 (
    echo [ERROR] ExpressDesigner build failed
    exit /b 1
)

echo.
echo ============================================================
echo   BUILD SUCCESSFUL
echo ============================================================
if exist "%BUILD_DIR%\src\ExpressDesigner.exe" (
    echo Executable: %BUILD_DIR%\src\ExpressDesigner.exe
) else (
    echo WARNING: Searching for binary...
    dir /s /b "%BUILD_DIR%\ExpressDesigner.exe" 2>nul
)
echo.
echo Next steps:
echo   First time: deploy_app.bat
echo   Rebuild:    build_reliable.bat noclean
endlocal