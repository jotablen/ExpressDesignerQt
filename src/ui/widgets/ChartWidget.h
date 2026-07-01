#pragma once
#include <QWidget>
#ifdef HAS_QT_CHARTS
#include <QtCharts/QChart>
#endif
#include <core/Project.h>

namespace ExpressDesigner {

class ChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChartWidget(QWidget* parent = nullptr) : QWidget(parent) {}

#ifdef HAS_QT_CHARTS
    static QColor objectColor(CustomObject* obj);
    static void populateChart(QChart* chart, Project* project,
                              bool showControlPoints, bool showNormals,
                              CustomObject* selectedObject = nullptr, bool alignLegendRight = true,
                              int normalsCount = 10);

private:
    // Internal helpers — each adds series/axes to chart, updates bounding rect
    struct Bounds { double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9; };
    static bool addObjectSeries(QChart* chart, CustomObject* obj,
                                 CustomObject* selectedObject,
                                 bool showControlPoints, Bounds& bounds);
    static void addNormalArrows(QChart* chart, Project* project,
                                 CustomObject* selectedObject, Bounds& bounds, int normalsCount);
    static void setupAxes(QChart* chart, const Bounds& bounds);
#else
    static void populateChart(void* chart, Project* project,
                              bool showControlPoints, bool showNormals) {}
#endif
};

} // namespace ExpressDesigner