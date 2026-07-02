cmake_minimum_required(VERSION 3.20)
project(ExpressDesigner VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(USE_SISL "Use SISL geometry library" ON)
option(ENABLE_CAD_EXPORT "Enable OpenCASCADE-based CAD export (STEP/IGES)" ON)

# Qt6 Configuration
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

# Optional Qt Charts
find_package(Qt6 COMPONENTS Charts QUIET)
if(Qt6Charts_FOUND)
    add_compile_definitions(HAS_QT_CHARTS=1)
    set(HAS_QT_CHARTS ON CACHE BOOL "Qt Charts available")
    message(STATUS "Qt Charts found - chart visualization enabled")
else()
    message(STATUS "Qt Charts NOT found - chart visualization will use stub")
endif()

# Google Test
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "Google Test not found - downloading...")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# SISL Configuration
if(USE_SISL)
    find_package(SISL QUIET)
    if(SISL_FOUND)
        message(STATUS "Using system SISL package")
    else()
        message(STATUS "SISL not found as system package - using local build from third_party/sisl")
        add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/sisl ${CMAKE_BINARY_DIR}/sisl EXCLUDE_FROM_ALL)
    endif()
    add_compile_definitions(USE_SISL=1)
endif()

# Android-specific configuration
if(ANDROID)
    message(STATUS "=== Android build ===")
    message(STATUS "  ABI: ${ANDROID_ABI}")
    message(STATUS "  Platform: ${ANDROID_PLATFORM}")
    if(ENABLE_CAD_EXPORT)
        message(STATUS "  CAD export: DISABLED (OpenCASCADE not supported on Android)")
        set(ENABLE_CAD_EXPORT OFF CACHE BOOL "CAD export disabled for Android" FORCE)
    endif()
    if(BUILD_TESTS)
        message(STATUS "  Unit tests: DISABLED for Android")
        set(BUILD_TESTS OFF CACHE BOOL "Tests disabled for Android" FORCE)
    endif()
endif()

# Pass ENABLE_CAD_EXPORT to subdirectory
set(ENABLE_CAD_EXPORT ${ENABLE_CAD_EXPORT} CACHE BOOL "CAD export" FORCE)

add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
