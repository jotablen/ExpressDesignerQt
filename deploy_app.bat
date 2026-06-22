@echo off
setlocal enabledelayedexpansion

set "PROJECT_DIR=%~dp0"
set "PROJECT_DIR=%PROJECT_DIR:~0,-1%"
set "BIN_DIR=%PROJECT_DIR%\bin"
set "BUILD_EXE=%PROJECT_DIR%\build\src\ExpressDesigner.exe"
set "MINGW_BIN=C:\msys64\mingw64\bin"
set "MINGW_SHARE=C:\msys64\mingw64\share"
set "MINGW_LIB=C:\msys64\mingw64\lib"

echo ============================================================
echo   ExpressDesigner - Post-Build Deployment
echo ============================================================
echo.

if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%BIN_DIR%\platforms" mkdir "%BIN_DIR%\platforms"
if not exist "%BIN_DIR%\styles" mkdir "%BIN_DIR%\styles"

:: Step 1: Verify executable
if not exist "%BUILD_EXE%" (
    echo [ERROR] Build output not found
    exit /b 1
)
echo [OK] Build output found.

:: Step 2: Copy exe
echo.
echo [---] Copying executable...
robocopy "%PROJECT_DIR%\build\src" "%BIN_DIR%" ExpressDesigner.exe /is /r:1 /w:1 >nul 2>&1
if %errorlevel% gtr 7 copy /y "%BUILD_EXE%" "%BIN_DIR%\" >nul 2>&1
echo [OK] ExpressDesigner.exe

:: Step 3: Qt6
echo.
echo [---] Copying Qt6 DLLs...
for %%d in (Qt6Core Qt6Gui Qt6Widgets Qt6Charts Qt6OpenGL Qt6OpenGLWidgets) do (
    if exist "%MINGW_BIN%\%%d.dll" (
        copy /y "%MINGW_BIN%\%%d.dll" "%BIN_DIR%\" >nul 2>&1
        echo [OK] %%d.dll
    )
)

:: Step 4: GCC/MinGW runtime
echo.
echo [---] Copying GCC runtime DLLs...
for %%d in (libgcc_s_seh-1 libstdc++-6 libwinpthread-1 libdouble-conversion libb2-1 libicuin78 libicuuc78 libicudt78 libpcre2-16-0 libpcre2-8-0 zlib1 libzstd libfreetype-6 libharfbuzz-0 libmd4c libpng16-16 libbrotlidec libbrotlicommon libbz2-1 libglib-2.0-0 libgraphite2 libintl-8 libiconv-2) do (
    if exist "%MINGW_BIN%\%%d.dll" (
        copy /y "%MINGW_BIN%\%%d.dll" "%BIN_DIR%\" >nul 2>&1
        echo [OK] %%d.dll
    )
)

:: Step 5: Platform plugin
echo.
echo [---] Copying platform plugin...
if exist "%MINGW_SHARE%\qt6\plugins\platforms\qwindows.dll" (
    copy /y "%MINGW_SHARE%\qt6\plugins\platforms\qwindows.dll" "%BIN_DIR%\platforms\" >nul 2>&1
    echo [OK] platforms\qwindows.dll
) else if exist "%MINGW_LIB%\qt6\plugins\platforms\qwindows.dll" (
    copy /y "%MINGW_LIB%\qt6\plugins\platforms\qwindows.dll" "%BIN_DIR%\platforms\" >nul 2>&1
    echo [OK] platforms\qwindows.dll
) else (
    echo [ERROR] qwindows.dll NOT FOUND
)

:: Step 6: Recursive DLL resolution via PowerShell
echo.
echo [---] Resolving transitive dependencies...
powershell -NoProfile -ExecutionPolicy Bypass -File "%PROJECT_DIR%\tools\resolve_deps.ps1" -ExePath "%BUILD_EXE%" -MingwBin "%MINGW_BIN%" -BinDir "%BIN_DIR%"

:: Step 7: Styles
if exist "%MINGW_SHARE%\qt6\plugins\styles\qwindowsvistastyle.dll" (
    copy /y "%MINGW_SHARE%\qt6\plugins\styles\qwindowsvistastyle.dll" "%BIN_DIR%\styles\" >nul 2>&1
    echo [OK] styles\qwindowsvistastyle.dll
)

:: Step 8: Backup libgcc
if exist "%PROJECT_DIR%\libgcc_s_seh-1.dll" (
    copy /y "%PROJECT_DIR%\libgcc_s_seh-1.dll" "%BIN_DIR%\" >nul 2>&1
)

:: Step 9: Launcher
echo.
echo [---] Creating launcher...
(
    echo @echo off
    echo cd /d "%%~dp0"
    echo start "" "%%~dp0ExpressDesigner.exe"
) > "%BIN_DIR%\run_ExpressDesigner.bat"
echo [OK] run_ExpressDesigner.bat

echo.
echo ============================================================
echo   Deployment Complete^^!
echo ============================================================
echo   Output: %BIN_DIR%
echo ============================================================
endlocal