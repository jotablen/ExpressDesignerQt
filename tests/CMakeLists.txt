cmake_minimum_required(VERSION 3.20)

# Core Tests
add_executable(test_core
    test_core/test_BaseObject.cpp
    test_core/test_CustomObject.cpp
    test_core/test_PointObject.cpp
    test_core/test_LineObject.cpp
    test_core/test_ArcObject.cpp
    test_core/test_CurveObject.cpp
    test_core/test_Project.cpp
    test_core/test_ObjectFactory.cpp
    test_core/test_ObjectTypes.cpp
    ${CMAKE_SOURCE_DIR}/src/core/BaseObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CustomObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/PointObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/LineObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ArcObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CurveObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Project.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CustomOperation.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CarthesianOvalOperation.cpp
    ${CMAKE_SOURCE_DIR}/src/core/PropagateWFOperation.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ObjectFactory.cpp
)
target_include_directories(test_core PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/geometry
    ${CMAKE_SOURCE_DIR}/src/optics
    ${CMAKE_SOURCE_DIR}/src/io
    ${CMAKE_SOURCE_DIR}/src/utils
)
target_link_libraries(test_core PRIVATE
    gtest
    gtest_main
    Qt6::Core
    Qt6::Gui
)

# Geometry Tests
add_executable(test_geometry
    test_geometry/test_SISLWrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/geometry/SISLWrapper.cpp
)
target_include_directories(test_geometry PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_geometry PRIVATE gtest gtest_main Qt6::Core Qt6::Gui)

# Optics Tests
add_executable(test_optics
    test_optics/test_SnellLaw.cpp
    test_optics/test_CartesianOval.cpp
    test_optics/test_OpticalPathLength.cpp
    ${CMAKE_SOURCE_DIR}/src/optics/SnellLaw.cpp
    ${CMAKE_SOURCE_DIR}/src/optics/CartesianOval.cpp
    ${CMAKE_SOURCE_DIR}/src/optics/OpticalPathLength.cpp
)
target_include_directories(test_optics PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_optics PRIVATE gtest gtest_main Qt6::Core Qt6::Gui)

# IO Tests
add_executable(test_io
    test_io/test_ProjectSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/io/ProjectSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/BaseObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CustomObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/PointObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/LineObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ArcObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CurveObject.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Project.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CustomOperation.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CarthesianOvalOperation.cpp
    ${CMAKE_SOURCE_DIR}/src/core/PropagateWFOperation.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ObjectFactory.cpp
)
target_include_directories(test_io PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/io
)
target_link_libraries(test_io PRIVATE gtest gtest_main Qt6::Core)

# Register tests
include(GoogleTest)
gtest_discover_tests(test_core)
gtest_discover_tests(test_geometry)
gtest_discover_tests(test_optics)
gtest_discover_tests(test_io)