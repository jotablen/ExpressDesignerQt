#!/bin/bash
set -e
export PATH="/mingw64/bin:/usr/bin:/c/Program Files/CMake/bin:$PATH"
cd /e/Projects/newVibes/ExpressDesignerQt

CLEAN=true
if [[ "$1" == "noclean" ]]; then
    CLEAN=false
    echo "=== Incremental build (no clean) ==="
else
    echo "=== Full build (clean) ==="
fi

if $CLEAN; then
    rm -rf build
    mkdir build
    cd build
    echo "=== Running CMake (USE_SISL=ON) ==="
    "/c/Program Files/CMake/bin/cmake.exe" .. \
      -G "MinGW Makefiles" \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTS=OFF \
      -DUSE_SISL=ON \
      -DCMAKE_PREFIX_PATH=/mingw64 \
      -DCMAKE_MAKE_PROGRAM=/mingw64/bin/mingw32-make.exe
else
    cd build 2>/dev/null || { echo "build directory not found, forcing clean"; exit 1; }
fi

echo "=== Building ==="
/mingw64/bin/mingw32-make.exe -j4

echo "=== BUILD COMPLETED ==="
echo "Executable:"
ls -la src/ExpressDesigner.exe 2>&1

echo ""
echo "Now run: deploy_app.bat"