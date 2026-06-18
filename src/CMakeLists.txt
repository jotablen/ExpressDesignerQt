set(CORE_SOURCES
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
    target_link_libraries(ExpressDesigner PRIVATE sisl)
endif()