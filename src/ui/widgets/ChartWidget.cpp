#include "ChartWidget.h"
#ifdef HAS_QT_CHARTS
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <core/CustomObject.h>
#include <core/PointObject.h>
#include <core/LineObject.h>
#include <core/ArcObject.h>
#include <core/CurveObject.h>
#include <core/ObjectTypes.h>

namespace ExpressDesigner {

// Color mapping: each base object type gets a distinct color
// Point = blue, Line = green, Arc = red/orange, Curve = purple
// WF variants use warmer shades
// Result variants use slightly darker shades
QColor ChartWidget::objectColor(CustomObject* obj)
{
    if (!obj) return QColor(128, 128, 128);

    uint16_t base = toBaseType(obj->objectType());
    bool isWF = obj->isWavefront();
    bool isRes = isResult(obj->objectType());

    // Base colors per object type
    if (base == 0x001 && isWF) return QColor(220, 50, 50);   // Point WF - Red
    if (base == 0x001) return QColor(50, 50, 220);            // Point - Blue
    if (base == 0x002 && isWF) return QColor(200, 100, 0);   // Line WF - Orange
    if (base == 0x002) return QColor(0, 150, 50);             // Line - Green
    if (base == 0x003 && isWF) return QColor(200, 50, 100);  // Arc WF - Magenta
    if (base == 0x003) return QColor(200, 80, 0);            // Arc - Dark Orange
    if (base == 0x004 && isWF) return QColor(180, 60, 180);  // Curve WF - Violet
    if (base == 0x004) return QColor(80, 0, 150);            // Curve - Purple

    return QColor(100, 100, 100);
}

void ChartWidget::populateChart(QChart* chart, Project* project,
                                 bool showControlPoints, bool showNormals,
                                 CustomObject* selectedObject, bool alignLegendRight)
{
    if (!chart || !project) return;
    chart->removeAllSeries();

    // Remove all axes too (removeAllSeries keeps existing axes)
    const auto axes = chart->axes();
    for (auto* axis : axes)
        chart->removeAxis(axis);

    if (alignLegendRight) {
        chart->legend()->setAlignment(Qt::AlignRight);
    }

    bool addedAny = false;
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;

    // Helper: get the effective pen color for an object
    auto effectivePen = [selectedObject](CustomObject* obj) -> QColor {
        if (obj == selectedObject) return QColor(255, 215, 0);  // Gold/Yellow
        return objectColor(obj);
    };

    auto addCurve = [&](CustomObject* obj) {
        if (!obj || !obj->isVisible()) return;
        const auto& pts = obj->controlPoints();
        if (pts.isEmpty()) return;

        QColor color = effectivePen(obj);
        bool isSelected = (obj == selectedObject);
        int penWidth = isSelected ? 3 : 2;

        // Main curve series (shown in legend)
        auto* series = new QLineSeries();
        series->setName(obj->name());
        series->setPen(QPen(color, penWidth));
        for (const auto& p : pts) {
            series->append(p.x(), p.y());
            minX = qMin(minX, p.x()); maxX = qMax(maxX, p.x());
            minY = qMin(minY, p.y()); maxY = qMax(maxY, p.y());
        }
        chart->addSeries(series);
        addedAny = true;

        // Control points as scatter (hidden from legend)
        if (showControlPoints && pts.size() > 1) {
            auto* scatter = new QScatterSeries();
            scatter->setName(QString());            // empty name = hidden from legend
            scatter->setMarkerSize(isSelected ? 8 : 6);
            scatter->setColor(color);
            scatter->setBorderColor(color.darker(130));
            for (const auto& p : pts)
                scatter->append(p.x(), p.y());
            chart->addSeries(scatter);
        }
    };

    for (auto* obj : project->dataObjects())
        addCurve(obj);
    for (auto* obj : project->resultObjects())
        addCurve(obj);

    // --- Draw normals if enabled ---
    if (showNormals) {
        for (auto* obj : project->allObjects()) {
            if (!obj || !obj->isVisible()) continue;
            auto normals = obj->computeNormals();
            QColor color = effectivePen(obj);
            for (const auto& pair : normals) {
                auto* arrow = new QLineSeries();
                arrow->setName(QString());           // hidden from legend
                arrow->setPen(QPen(color, 1));
                arrow->append(pair.first.x(), pair.first.y());
                arrow->append(pair.second.x(), pair.second.y());
                chart->addSeries(arrow);
                addedAny = true;
                minX = qMin(minX, qMin(pair.first.x(), pair.second.x()));
                maxX = qMax(maxX, qMax(pair.first.x(), pair.second.x()));
                minY = qMin(minY, qMin(pair.first.y(), pair.second.y()));
                maxY = qMax(maxY, qMax(pair.first.y(), pair.second.y()));
            }
        }
    }

    if (addedAny) {
        // Add slight padding
        double dx = (maxX - minX) * 0.1;
        double dy = (maxY - minY) * 0.1;
        if (qFuzzyCompare(dx, 0.0)) dx = 1.0;
        if (qFuzzyCompare(dy, 0.0)) dy = 1.0;

        auto* axisX = new QValueAxis();
        axisX->setRange(minX - dx, maxX + dx);
        axisX->setLabelFormat(QStringLiteral("%.1f"));
        chart->addAxis(axisX, Qt::AlignBottom);

        auto* axisY = new QValueAxis();
        axisY->setRange(minY - dy, maxY + dy);
        axisY->setLabelFormat(QStringLiteral("%.1f"));
        chart->addAxis(axisY, Qt::AlignLeft);

        for (auto* series : chart->series()) {
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }
    }
}

} // namespace ExpressDesigner
#endif // HAS_QT_CHARTS