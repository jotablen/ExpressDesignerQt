@echo off
setlocal enabledelayedexpansion

:: ============================================================
:: ExpressDesigner - Post-Build Deployment Script (Windows)
:: Copies the executable and all required DLLs to bin/
:: Run after: do_build.bat
:: ============================================================

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

:: Ensure destination dirs exist
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%BIN_DIR%\platforms" mkdir "%BIN_DIR%\platforms"
if not exist "%BIN_DIR%\styles" mkdir "%BIN_DIR%\styles"

:: ============================================================
:: Step 1: Verify the executable exists
:: ============================================================
if not exist "%BUILD_EXE%" (
    echo [ERROR] Build output not found: %BUILD_EXE%
    echo Run do_build.bat first to compile the project.
    exit /b 1
)
echo [OK] Build output found.

:: ============================================================
:: Step 2: Copy files preserving timestamps via robocopy
:: ============================================================
echo.
echo [---] Copying executable (preserving timestamp^)...

:: robocopy preserves timestamps by default; exit codes 0-7 are success
set "ROBOCOPY_ERR=0"
robocopy "%PROJECT_DIR%\build\src" "%BIN_DIR%" ExpressDesigner.exe /is /r:1 /w:1 >nul 2>&1
if %errorlevel% gtr 7 (
    echo [WARN] robocopy failed, trying xcopy...
    xcopy /d /y "%BUILD_EXE%" "%BIN_DIR%\" >nul 2>&1
    if %errorlevel% neq 0 (
        copy /y "%BUILD_EXE%" "%BIN_DIR%\ExpressDesigner.exe" >nul 2>&1
    )
)
echo [OK] ExpressDesigner.exe copied.

:: ============================================================
:: Step 3: Copy required Qt6 DLLs
:: ============================================================
echo.
echo [---] Copying Qt6 DLLs...
set "QT_DLLS=Qt6Core.dll Qt6Gui.dll Qt6Widgets.dll Qt6Charts.dll Qt6OpenGL.dll Qt6OpenGLWidgets.dll"
set "QT_MISSING="

for %%d in (%QT_DLLS%) do (
    if exist "%MINGW_BIN%\%%d" (
        copy /y "%MINGW_BIN%\%%d" "%BIN_DIR%\%%d" >nul 2>&1
        echo [OK] %%d
    ) else (
        set "QT_MISSING=!QT_MISSING! %%d"
        echo [WARN] %%d NOT FOUND in %MINGW_BIN%
    )
)
if not "!QT_MISSING!"=="" (
    echo [WARN] Some Qt DLLs were not found: !QT_MISSING!
    echo        The application may fail to start.
)

:: ============================================================
:: Step 4: Copy GCC/MinGW runtime DLLs
:: ============================================================
echo.
echo [---] Copying GCC runtime DLLs...
set "GCC_DLLS=libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll libdouble-conversion.dll libb2-1.dll libicuin78.dll libicuuc78.dll libicudt78.dll libpcre2-16-0.dll libpcre2-8-0.dll zlib1.dll libzstd.dll libfreetype-6.dll libharfbuzz-0.dll libmd4c.dll libpng16-16.dll libbrotlidec.dll libbrotlicommon.dll libbz2-1.dll libglib-2.0-0.dll libgraphite2.dll libintl-8.dll libiconv-2.dll"
set "GCC_MISSING="

for %%d in (%GCC_DLLS%) do (
    if exist "%MINGW_BIN%\%%d" (
        copy /y "%MINGW_BIN%\%%d" "%BIN_DIR%\%%d" >nul 2>&1
        echo [OK] %%d
    ) else (
        set "GCC_MISSING=!GCC_MISSING! %%d"
        echo [WARN] %%d NOT FOUND in %MINGW_BIN%
    )
)

:: ============================================================
:: Step 5: Copy Qt6 platform plugin (critical for GUI)
:: ============================================================
echo.
echo [---] Copying platform plugins...
set "PLATFORM_COPIED=0"
if exist "%MINGW_SHARE%\qt6\plugins\platforms\qwindows.dll" (
    copy /y "%MINGW_SHARE%\qt6\plugins\platforms\qwindows.dll" "%BIN_DIR%\platforms\qwindows.dll" >nul 2>&1
    echo [OK] platforms\qwindows.dll
    set "PLATFORM_COPIED=1"
)
if "!PLATFORM_COPIED!"=="0" (
    if exist "%MINGW_LIB%\qt6\plugins\platforms\qwindows.dll" (
        copy /y "%MINGW_LIB%\qt6\plugins\platforms\qwindows.dll" "%BIN_DIR%\platforms\qwindows.dll" >nul 2>&1
        echo [OK] platforms\qwindows.dll (from lib^)
        set "PLATFORM_COPIED=1"
    )
)
if "!PLATFORM_COPIED!"=="0" (
    echo [ERROR] qwindows.dll NOT FOUND - The application will crash on startup.
    echo        Searched in:
    echo         - %MINGW_SHARE%\qt6\plugins\platforms\
    echo         - %MINGW_LIB%\qt6\plugins\platforms\
)

:: ============================================================
:: Step 6: Copy Qt6 styles plugin (optional)
:: ============================================================
if exist "%MINGW_SHARE%\qt6\plugins\styles\qwindowsvistastyle.dll" (
    copy /y "%MINGW_SHARE%\qt6\plugins\styles\qwindowsvistastyle.dll" "%BIN_DIR%\styles\qwindowsvistastyle.dll" >nul 2>&1
    echo [OK] styles\qwindowsvistastyle.dll
)

:: ============================================================
:: Step 7: Copy libgcc_s_seh-1.dll from project root (backup)
:: ============================================================
if exist "%PROJECT_DIR%\libgcc_s_seh-1.dll" (
    copy /y "%PROJECT_DIR%\libgcc_s_seh-1.dll" "%BIN_DIR%\libgcc_s_seh-1.dll" >nul 2>&1
    echo [OK] libgcc_s_seh-1.dll (from project root^)
)

:: ============================================================
:: Step 8: Generate a convenience launcher
:: ============================================================
echo.
echo [---] Creating launch script...
set "LAUNCHER=%BIN_DIR%\run_ExpressDesigner.bat"
(
    echo @echo off
    echo cd /d "%%~dp0"
    echo start "" "%%~dp0ExpressDesigner.exe"
) > "%LAUNCHER%"
echo [OK] run_ExpressDesigner.bat created.

:: ============================================================
:: Summary
:: ============================================================
echo.
echo ============================================================
echo   Deployment Complete^^!
echo ============================================================
echo.
echo   Output: %BIN_DIR%
echo   Executable: ExpressDesigner.exe
echo   Launcher: run_ExpressDesigner.bat
echo.
echo   Deployed DLLs:
echo     - Qt6Core.dll
echo     - Qt6Gui.dll
echo     - Qt6Widgets.dll
echo     - Qt6Charts.dll
echo     - libgcc_s_seh-1.dll
echo     - libstdc++-6.dll
if exist "%BIN_DIR%\libwinpthread-1.dll" echo     - libwinpthread-1.dll
echo     - platforms\qwindows.dll
echo.
echo   To distribute this application, include the entire
echo   bin\ directory. Double-click run_ExpressDesigner.bat
echo   or launch ExpressDesigner.exe directly.
echo.
if not "!QT_MISSING!"=="" (
    echo   *** WARNING: Some Qt DLLs were missing: !QT_MISSING!
    echo   *** The application may not start properly.
)
echo ============================================================
endlocal