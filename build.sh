#!/bin/bash
echo "=== STARTING BUILD ==="
export PATH=/mingw64/bin:/usr/bin:$PATH
echo "PATH=$PATH"
cd /e/Projects/newVibes/ExpressDesignerQt
rm -rf build
mkdir build
cd build
echo "--- CMake Configure ---"
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DUSE_SISL=OFF -DCMAKE_PREFIX_PATH=/mingw64 2>&1
echo "CMake exit code: $?"
echo "--- CMake Build ---"
cmake --build . -- -j4 2>&1
echo "Build exit code: $?"
echo "--- Checking binary ---"
ls -la ExpressDesigner.exe 2>&1 || echo "No binary found"
echo "=== BUILD FINISHED ==="