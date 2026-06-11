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
    if(NOT SISL_FOUND)
        message(STATUS "SISL not found as system package - will build from source")
        set(SISL_BUILD_DIR ${CMAKE_BINARY_DIR}/sisl)
        # SISL stub for now - actual SISL integration via FetchContent
        include(FetchContent)
        FetchContent_Declare(
            sisl
            GIT_REPOSITORY https://github.com/SINTEF-Geometry/SISL.git
            GIT_TAG master
        )
        # Note: SISL is a C library; wrap appropriately
    endif()
    add_compile_definitions(USE_SISL=1)
endif()

add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()