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
                              CustomObject* selectedObject = nullptr, bool alignLegendRight = true);
#else
    static void populateChart(void* chart, Project* project,
                              bool showControlPoints, bool showNormals) {}
#endif
};

} // namespace ExpressDesigner