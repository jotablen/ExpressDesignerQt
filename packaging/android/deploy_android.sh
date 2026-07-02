#!/bin/bash
# deploy_android.sh — Package and deploy ExpressDesigner APK for Android
#
# Prerequisites:
#   - build_android.sh must have been run successfully first
#   - ANDROID_SDK_ROOT, ANDROID_NDK_ROOT, QT_ANDROID, JAVA_HOME must be set
#
# Usage:
#   export ANDROID_NDK_ROOT=/path/to/ndk
#   export ANDROID_SDK_ROOT=/path/to/sdk
#   export QT_ANDROID=/path/to/qt/android_arm64_v8a
#   export JAVA_HOME=/path/to/jdk17
#   ./packaging/android/deploy_android.sh
#
# Optional env vars:
#   ANDROID_KEYSTORE  — path to release keystore (.jks)
#   ANDROID_KEYALIAS  — key alias in the keystore
#   ANDROID_KEYPASS   — password for the key
#   ANDROID_STOREPASS — password for the keystore

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build_android"
DEPLOY_DIR="${BUILD_DIR}/deploy"
APK_OUTPUT_DIR="${PROJECT_ROOT}/bin/android"

# -------- Environment checks --------
check_var() {
    local name=$1 val=${2:-}
    if [ -z "$val" ]; then
        echo "[ERROR] $name is not set." >&2
        echo "  Export it before running this script." >&2
        exit 1
    fi
}

check_var ANDROID_NDK_ROOT "${ANDROID_NDK_ROOT:-}"
check_var ANDROID_SDK_ROOT "${ANDROID_SDK_ROOT:-}"
check_var QT_ANDROID        "${QT_ANDROID:-}"
check_var JAVA_HOME         "${JAVA_HOME:-}"

# -------- Find the native library --------
LIB_PATH=$(find "${BUILD_DIR}" -name "libExpressDesigner_*.so" 2>/dev/null | head -1)
if [ -z "$LIB_PATH" ]; then
    LIB_PATH=$(find "${BUILD_DIR}" -name "libExpressDesigner.so" 2>/dev/null | head -1)
fi
if [ -z "$LIB_PATH" ]; then
    echo "[ERROR] Native library not found. Run build_android.sh first." >&2
    exit 1
fi

ANDROID_ABI=$(echo "${LIB_PATH}" | grep -oE '(arm64-v8a|armeabi-v7a|x86|x86_64)' || echo "arm64-v8a")
echo "Detected ABI: ${ANDROID_ABI}"

# -------- Copy library to packaging --------
echo "[1/5] Preparing deploy layout..."
mkdir -p "${SCRIPT_DIR}/libs/${ANDROID_ABI}"
cp "${LIB_PATH}" "${SCRIPT_DIR}/libs/${ANDROID_ABI}/"

# Also copy SISL if it was built as a separate shared library
SISL_LIB=$(find "${BUILD_DIR}" -name "libsisl*.so" 2>/dev/null | head -1)
if [ -n "$SISL_LIB" ]; then
    echo "  Copying SISL library..."
    cp "${SISL_LIB}" "${SCRIPT_DIR}/libs/${ANDROID_ABI}/"
fi

# -------- androiddeployqt --------
echo "[2/5] Running androiddeployqt..."
mkdir -p "${DEPLOY_DIR}"

# Build the JSON config for androiddeployqt
DEPLOY_JSON="${DEPLOY_DIR}/deploy-settings.json"
cat > "${DEPLOY_JSON}" << EOJSON
{
    "description": "ExpressDesigner Android deployment",
    "qt": "${QT_ANDROID}",
    "sdk": "${ANDROID_SDK_ROOT}",
    "ndk": "${ANDROID_NDK_ROOT}",
    "stdcpp": "c++_shared",
    "sdkBuildToolsRevision": "34.0.0",
    "android-package-source-directory": "${SCRIPT_DIR}",
    "android-extra-libs": "",
    "android-min-sdk-version": "21",
    "android-target-sdk-version": "34",
    "useGradle": true,
    "install": false
}
EOJSON

# Find androiddeployqt
ADQ="${QT_ANDROID}/bin/androiddeployqt"
if [ ! -x "$ADQ" ]; then
    # Try Qt Installer layout
    ADQ="${QT_ANDROID}/../../Tools/QtCreator/bin/androiddeployqt"
fi
if [ ! -x "$ADQ" ]; then
    echo "[ERROR] androiddeployqt not found at ${ADQ}" >&2
    exit 1
fi

export PATH="${JAVA_HOME}/bin:${ANDROID_SDK_ROOT}/platform-tools:${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin:${ANDROID_SDK_ROOT}/build-tools/34.0.0:${PATH}"

"${ADQ}" \
    --input "${DEPLOY_JSON}" \
    --output "${DEPLOY_DIR}" \
    --apk "${APK_OUTPUT_DIR}/ExpressDesigner-debug.apk" \
    --verbose

# -------- Generate release APK if keystore provided --------
if [ -n "${ANDROID_KEYSTORE:-}" ] && [ -f "${ANDROID_KEYSTORE}" ]; then
    echo "[3/5] Signing release APK..."
    APK_RELEASE="${APK_OUTPUT_DIR}/ExpressDesigner-release.apk"

    "${ADQ}" \
        --input "${DEPLOY_JSON}" \
        --output "${DEPLOY_DIR}" \
        --sign "${ANDROID_KEYSTORE}" "${ANDROID_KEYALIAS:-expressdesigner}" \
        --storepass "${ANDROID_STOREPASS:-}" \
        --keypass "${ANDROID_KEYPASS:-}" \
        --apk "${APK_RELEASE}" \
        --release \
        --verbose

    echo "  Release APK: ${APK_RELEASE}"
else
    echo "[3/5] Skipping release APK (no keystore provided)"
    echo "  Set ANDROID_KEYSTORE, ANDROID_KEYALIAS, ANDROID_STOREPASS, ANDROID_KEYPASS for release."
fi

# -------- Show output --------
echo "[4/5] Build artifacts:"
find "${APK_OUTPUT_DIR}" -name "*.apk" -exec ls -lh {} \;

echo ""
echo "[5/5] Installing on connected device..."
ADB="${ANDROID_SDK_ROOT}/platform-tools/adb"
if [ -x "$ADB" ]; then
    DEVICE=$("$ADB" devices 2>/dev/null | grep -v "List" | grep "device" | head -1 | awk '{print $1}')
    if [ -n "$DEVICE" ]; then
        "$ADB" -s "$DEVICE" install -r "${APK_OUTPUT_DIR}/ExpressDesigner-debug.apk" && \
            echo "" && \
            echo "============================================================" && \
            echo "  DEPLOYMENT SUCCESSFUL" && \
            echo "============================================================" && \
            echo "  App installed on device: ${DEVICE}" && \
            echo "  APK: ${APK_OUTPUT_DIR}/ExpressDesigner-debug.apk" && \
            echo "============================================================"
    else
        echo "  No device/emulator connected. APK available at:"
        echo "  ${APK_OUTPUT_DIR}/ExpressDesigner-debug.apk"
    fi
else
    echo "  adb not found. APK available at:"
    echo "  ${APK_OUTPUT_DIR}/ExpressDesigner-debug.apk"
fi