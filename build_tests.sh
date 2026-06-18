#!/bin/bash
set -e
cd /e/Projects/newVibes/ExpressDesignerQt
rm -rf build_test
mkdir build_test
cd build_test
cmake .. -G "MinGW Makefiles" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DUSE_SISL=OFF \
  -DCMAKE_PREFIX_PATH=/mingw64 2>&1
cmake --build . -- -j4 2>&1
echo "=== BUILD COMPLETED ==="
echo ""
echo "=== Running SISLWrapper test ==="
./tests/test_geometry/test_geometry.exe 2>&1
echo ""
echo "=== Running Core tests ==="
./tests/test_core/test_core.exe 2>&1