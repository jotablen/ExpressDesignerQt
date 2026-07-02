#!/bin/bash
# build_android.sh — Build ExpressDesigner for Android
#
# Prerequisites:
#   - Android NDK (r25+ recommended)  → $ANDROID_NDK_ROOT
#   - Android SDK (API 34)            → $ANDROID_SDK_ROOT
#   - Qt 6.x for Android             → $QT_ANDROID (e.g. /opt/Qt/6.8.0/android_arm64_v8a)
#   - JDK 17                          → $JAVA_HOME
#
# Usage:
#   export ANDROID_NDK_ROOT=/path/to/ndk
#   export ANDROID_SDK_ROOT=/path/to/sdk
#   export QT_ANDROID=/path/to/qt/android_arm64_v8a
#   export JAVA_HOME=/path/to/jdk17
#   ./build_android.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build_android"
PACKAGING_DIR="${SCRIPT_DIR}/packaging/android"

# -------- Environment checks --------
check_var() {
    local name=$1 val=${2:-}
    if [ -z "$val" ]; then
        echo "[ERROR] $name is not set." >&2
        echo "  Export it before running this script." >&2
        exit 1
    fi
}

: "${ANDROID_ABI:=arm64-v8a}"
: "${ANDROID_PLATFORM:=android-21}"
: "${BUILD_TYPE:=Release}"

check_var ANDROID_NDK_ROOT "${ANDROID_NDK_ROOT:-}"
check_var ANDROID_SDK_ROOT "${ANDROID_SDK_ROOT:-}"
check_var QT_ANDROID        "${QT_ANDROID:-}"
check_var JAVA_HOME         "${JAVA_HOME:-}"

# -------- Config --------
CMAKE="${ANDROID_SDK_ROOT}/cmake/3.22.1/bin/cmake"
if [ ! -x "$CMAKE" ]; then
    # Try system cmake (works if NDK toolchain can be specified)
    CMAKE="cmake"
fi

TOOLCHAIN="${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake"
if [ ! -f "$TOOLCHAIN" ]; then
    echo "[ERROR] Android toolchain not found at $TOOLCHAIN" >&2
    exit 1
fi

echo "============================================================"
echo "  ExpressDesigner — Android Build"
echo "============================================================"
echo "  ABI:            ${ANDROID_ABI}"
echo "  Platform:       ${ANDROID_PLATFORM}"
echo "  Build type:     ${BUILD_TYPE}"
echo "  NDK:            ${ANDROID_NDK_ROOT}"
echo "  SDK:            ${ANDROID_SDK_ROOT}"
echo "  Qt Android:     ${QT_ANDROID}"
echo "  Java:           ${JAVA_HOME}"
echo "============================================================"

# -------- Clean previous build --------
echo "[1/4] Preparing build directory..."
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# -------- CMake configure --------
echo "[2/4] Configuring CMake..."
export PATH="${JAVA_HOME}/bin:${ANDROID_SDK_ROOT}/platform-tools:${PATH}"

"${CMAKE}" \
    -B "${BUILD_DIR}" \
    -S "${SCRIPT_DIR}" \
    -G "Ninja" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
    -DANDROID_ABI="${ANDROID_ABI}" \
    -DANDROID_PLATFORM="${ANDROID_PLATFORM}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH="${QT_ANDROID}" \
    -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ON \
    -DBUILD_TESTS=OFF \
    -DUSE_SISL=ON \
    -DENABLE_CAD_EXPORT=OFF \
    -DQT_HOST_PATH="${QT_ANDROID}/../gcc_64" \
    2>&1 | tee "${BUILD_DIR}/cmake_configure.log"

echo "CMake exit code: $?"

# -------- Build --------
echo "[3/4] Compiling..."
"${CMAKE}" --build "${BUILD_DIR}" --parallel "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)" \
    2>&1 | tee "${BUILD_DIR}/build.log"

# -------- Find the library --------
LIB_PATH=$(find "${BUILD_DIR}" -name "libExpressDesigner_*.so" 2>/dev/null | head -1)
if [ -z "$LIB_PATH" ]; then
    LIB_PATH=$(find "${BUILD_DIR}" -name "libExpressDesigner.so" 2>/dev/null | head -1)
fi

if [ -z "$LIB_PATH" ]; then
    echo "[ERROR] Native library not found in ${BUILD_DIR}" >&2
    echo "  Searching recursively:"
    find "${BUILD_DIR}" -name "*.so" 2>/dev/null || true
    exit 1
fi

echo "[4/4] Native library: ${LIB_PATH}"
echo ""
echo "============================================================"
echo "  BUILD SUCCESSFUL"
echo "============================================================"
echo ""
echo "Next step: run ./packaging/android/deploy_android.sh"