#!/bin/bash
set -e
cd /e/Projects/newVibes/ExpressDesignerQt
rm -rf build
mkdir build
cd build
cmake .. -G "MinGW Makefiles" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=OFF \
  -DUSE_SISL=OFF \
  -DCMAKE_PREFIX_PATH=/mingw64 2>&1
cmake --build . -- -j4 2>&1
echo "=== BUILD COMPLETED ==="
ls -la ExpressDesigner.exe 2>&1 || echo "Binary not found, listing directory:"
ls -la 2>&1