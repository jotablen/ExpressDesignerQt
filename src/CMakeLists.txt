set(CORE_SOURCES
    core/RefPointDescriptor.cpp
    core/BaseObject.cpp
    core/CustomObject.cpp
    core/PointObject.cpp
    core/LineObject.cpp
    core/ArcObject.cpp
    core/CurveObject.cpp
    core/Project.cpp
    core/CustomOperation.cpp
    core/CarthesianOvalOperation.cpp
    core/PropagateWFOperation.cpp
    core/ObjectFactory.cpp
    core/DependencyGraph.cpp
    core/CommandHistory.cpp
)

set(GEOMETRY_SOURCES
    geometry/SISLWrapper.cpp
    geometry/PointList.cpp
    geometry/VectorAlgebra.cpp
)

set(OPTICS_SOURCES
    optics/CartesianOval.cpp
    optics/WavefrontPropagator.cpp
    optics/FresnelLensCalculator.cpp
    optics/SnellLaw.cpp
    optics/OpticalPathLength.cpp
)

set(IO_SOURCES
    io/ProjectSerializer.cpp
    io/TXTExporter.cpp
    io/TXTImporter.cpp
    io/RhinoExporter.cpp
    io/CADExporter.cpp
    io/HistoryManager.cpp
)

set(UI_SOURCES
    ui/MainWindow.cpp
    ui/dialogs/SplashDialog.cpp
    ui/dialogs/AboutDialog.cpp
    ui/dialogs/InsertObjectDialog.cpp
    ui/dialogs/CalcOvalDialog.cpp
    ui/dialogs/PropagateWFDialog.cpp
    ui/dialogs/ExportObjectDialog.cpp
    ui/dialogs/ExportAllRhinoDialog.cpp
    ui/dialogs/ImportObjectDialog.cpp
    ui/dialogs/CurvePointsDialog.cpp
    ui/dialogs/EditPointDialog.cpp
    ui/dialogs/NormalsParamsDialog.cpp
    ui/dialogs/OffsetWFDialog.cpp
    ui/dialogs/SetScaleDialog.cpp
    ui/dialogs/ProjectHistoryDialog.cpp
    ui/dialogs/PreferencesDialog.cpp
    ui/dialogs/RotateObjectDialog.cpp
    ui/dialogs/TranslateObjectDialog.cpp
    ui/dialogs/DependencyGraphDialog.cpp
    ui/dialogs/CopyObjectDialog.cpp
    ui/dialogs/ExportCADDialog.cpp
    ui/widgets/ObjectTreeWidget.cpp
    ui/widgets/PropertiesWidget.cpp
    ui/widgets/ChartWidget.cpp
    ui/models/ObjectTreeModel.cpp
    ui/models/ObjectListModel.cpp
    ui/delegates/PropertyDelegate.cpp
)

set(APP_SOURCES
    app/OvalDesignerApp.cpp
)

set(UTILS_SOURCES
    utils/NumericUtils.cpp
    utils/StringUtils.cpp
    utils/Logger.cpp
)

# Main executable
add_executable(ExpressDesigner
    main.cpp
    ${CORE_SOURCES}
    ${GEOMETRY_SOURCES}
    ${OPTICS_SOURCES}
    ${IO_SOURCES}
    ${UI_SOURCES}
    ${APP_SOURCES}
    ${UTILS_SOURCES}
)

target_include_directories(ExpressDesigner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry
    ${CMAKE_CURRENT_SOURCE_DIR}/optics
    ${CMAKE_CURRENT_SOURCE_DIR}/io
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(ExpressDesigner PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(HAS_QT_CHARTS)
    target_link_libraries(ExpressDesigner PRIVATE Qt6::Charts)
endif()

if(USE_SISL AND TARGET sisl)
    target_include_directories(ExpressDesigner PRIVATE ${PROJECT_SOURCE_DIR}/third_party/sisl/include)
    target_link_libraries(ExpressDesigner PRIVATE sisl)
endif()

# OpenCASCADE (CAD export)
option(ENABLE_CAD_EXPORT "Enable OpenCASCADE-based CAD export (STEP/IGES)" ON)

if(ENABLE_CAD_EXPORT)
    if(WIN32)
        # Windows: MSYS2/MinGW — manually link against OCCT DLLs
        set(OCCT_ROOT "C:/msys64/mingw64")
        if(EXISTS "${OCCT_ROOT}/include/opencascade/Standard_Version.hxx")
            set(OCCT_INCLUDE_DIR "${OCCT_ROOT}/include/opencascade")
            set(OCCT_LIB_DIR "${OCCT_ROOT}/lib")
            set(OCCT_CORE_MODULES
                TKernel TKMath TKG2d TKG3d TKGeomBase TKGeomAlgo TKBRep
                TKTopAlgo TKPrim TKBO TKBool
                TKDESTEP TKDEIGES TKDE TKXSBase TKShHealing TKMesh
                TKCAF TKBinXCAF TKDCAF TKLCAF TKVCAF TKXCAF TKXmlXCAF
                tbb12
            )
            foreach(mod ${OCCT_CORE_MODULES})
                if(EXISTS "${OCCT_LIB_DIR}/lib${mod}.dll.a")
                    list(APPEND OCCT_LIBS "${OCCT_LIB_DIR}/lib${mod}.dll.a")
                endif()
            endforeach()
            if(OCCT_LIBS)
                target_include_directories(ExpressDesigner PRIVATE ${OCCT_INCLUDE_DIR})
                target_link_libraries(ExpressDesigner PRIVATE ${OCCT_LIBS})
                target_compile_definitions(ExpressDesigner PRIVATE HAS_OPENCASCADE)
                message(STATUS "OpenCASCADE found — CAD export enabled (${OCCT_LIBS})")
            endif()
        else()
            message(STATUS "OpenCASCADE not found — CAD export disabled")
        endif()
    elseif(APPLE)
        # macOS: Homebrew OpenCASCADE
        find_library(OCCT_TKERNEL NAMES TKernel PATHS /opt/homebrew/lib /usr/local/lib)
        if(OCCT_TKERNEL)
            execute_process(COMMAND brew --prefix opencascade
                OUTPUT_VARIABLE OCCT_PREFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
            if(NOT OCCT_PREFIX)
                find_path(OCCT_INCLUDE_DIR Standard_Version.hxx
                    PATHS /opt/homebrew/include/opencascade /usr/local/include/opencascade)
            else()
                set(OCCT_INCLUDE_DIR "${OCCT_PREFIX}/include/opencascade")
            endif()
            find_library(OCCT_LIBS
                NAMES
                    TKernel TKMath TKG2d TKG3d TKGeomBase TKGeomAlgo TKBRep
                    TKTopAlgo TKPrim TKBO TKBool
                    TKDESTEP TKDEIGES TKDE TKXSBase TKShHealing TKMesh
                    TKCAF TKBinXCAF TKDCAF TKLCAF TKVCAF TKXCAF TKXmlXCAF
                    tbb
                PATHS /opt/homebrew/lib /usr/local/lib)
            if(OCCT_LIBS AND OCCT_INCLUDE_DIR)
                target_include_directories(ExpressDesigner PRIVATE ${OCCT_INCLUDE_DIR})
                target_link_libraries(ExpressDesigner PRIVATE ${OCCT_LIBS})
                target_compile_definitions(ExpressDesigner PRIVATE HAS_OPENCASCADE)
                message(STATUS "OpenCASCADE found — CAD export enabled")
            else()
                message(STATUS "OpenCASCADE libraries not found — CAD export disabled")
            endif()
        else()
            message(STATUS "OpenCASCADE not found — CAD export disabled")
        endif()
    else()
        # Linux / other: use find_package as fallback
        find_package(OpenCASCADE QUIET)
        if(OpenCASCADE_FOUND)
            find_package(TBB QUIET)
            target_include_directories(ExpressDesigner PRIVATE ${OpenCASCADE_INCLUDE_DIR})
            target_link_libraries(ExpressDesigner PRIVATE
                TKBinXCAF TKBin TKBool TKBO TKBRep TKCAF TKCDF TKDCAF TKDE TKDESTEP
                TKDraw TKernel TKFeat TKFillet TKG2d TKG3d TKGeomAlgo TKGeomBase
                TKHLR TKIVtk TKLCAF TKMath TKMesh TKMeshVS TKOffset TKOpenGl
                TKPrim TKQADraw TKRWMesh TKService TKShHealing TKStd TKStdL
                TKTObj TKTopAlgo TKV3d TKVCAF TKViewerTest TKXCAF TKXDEDRAW
                TKXDEIGES TKXMesh TKXml TKXmlL TKXmlTObj TKXmlXCAF TKXSBase
                TKXSDRAW TKXSDRAWIGES ${TBB_LIBRARIES})
            target_compile_definitions(ExpressDesigner PRIVATE HAS_OPENCASCADE)
            message(STATUS "OpenCASCADE found — CAD export enabled")
        else()
            message(STATUS "OpenCASCADE not found — CAD export disabled")
        endif()
    endif()
else()
    message(STATUS "CAD export disabled (ENABLE_CAD_EXPORT=OFF)")
endif()
