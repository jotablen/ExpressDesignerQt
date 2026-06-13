#include "ChartWidget.h"
#ifdef HAS_QT_CHARTS
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegendMarker>
#include <core/CustomObject.h>
#include <core/PointObject.h>
#include <core/LineObject.h>
#include <core/ArcObject.h>
#include <core/CurveObject.h>
#include <core/ObjectTypes.h>

#include <map>
#include <utility>

namespace ExpressDesigner {

// Color mapping: each base object type gets a distinct color
// Point = blue, Line = green, Arc = red/orange, Curve = purple
// WF variants use warmer shades
QColor ChartWidget::objectColor(CustomObject* obj)
{
    if (!obj) return QColor(128, 128, 128);

    // Key: pair<baseType, isWavefront>
    static const std::map<std::pair<uint16_t, bool>, QColor> colorMap = {
        {{0x001, true},  QColor(220, 50, 50)},   // Point WF - Red
        {{0x001, false}, QColor(50, 50, 220)},    // Point - Blue
        {{0x002, true},  QColor(200, 100, 0)},   // Line WF - Orange
        {{0x002, false}, QColor(0, 150, 50)},     // Line - Green
        {{0x003, true},  QColor(200, 50, 100)},  // Arc WF - Magenta
        {{0x003, false}, QColor(200, 80, 0)},    // Arc - Dark Orange
        {{0x004, true},  QColor(180, 60, 180)},  // Curve WF - Violet
        {{0x004, false}, QColor(80, 0, 150)},    // Curve - Purple
    };

    uint16_t base = toBaseType(obj->objectType());
    bool isWF = obj->isWavefront();

    auto it = colorMap.find({base, isWF});
    if (it != colorMap.end())
        return it->second;

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
        series->setName(obj->name().isEmpty() ? QStringLiteral("Unnamed") : obj->name());
        series->setPen(QPen(color, penWidth));
        // Store pointer to CustomObject for hit-testing on click
        series->setProperty("customObject", QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
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
            scatter->setColor(color);
            scatter->setBorderColor(color.darker(130));
            scatter->setMarkerSize(isSelected ? 8 : 6);
            for (const auto& p : pts)
                scatter->append(p.x(), p.y());
            chart->addSeries(scatter);
            // Hide from legend via marker
            auto markers = chart->legend()->markers(scatter);
            for (auto* marker : markers)
                marker->setVisible(false);
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
                arrow->setPen(QPen(color, 1));
                arrow->append(pair.first.x(), pair.first.y());
                arrow->append(pair.second.x(), pair.second.y());
                chart->addSeries(arrow);
                // Hide from legend via marker
                auto markers = chart->legend()->markers(arrow);
                for (auto* marker : markers)
                    marker->setVisible(false);
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