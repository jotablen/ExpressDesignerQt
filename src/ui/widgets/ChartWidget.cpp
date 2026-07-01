#include "ChartWidget.h"
#ifdef HAS_QT_CHARTS
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
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

// ============================================================================
// populateChart — main entry point
// ============================================================================
void ChartWidget::populateChart(QChart* chart, Project* project,
                                 bool showControlPoints, bool showNormals,
                                 CustomObject* selectedObject, bool alignLegendRight,
                                 int normalsCount, double normalsLength)
{
    if (!chart || !project) return;

    // Suspend signal emissions while rebuilding (avoids redundant chart updates)
    chart->blockSignals(true);

    chart->removeAllSeries();
    // Remove all axes too (removeAllSeries keeps existing axes)
    const auto axes = chart->axes();
    for (auto* axis : axes)
        chart->removeAxis(axis);

    if (alignLegendRight)
        chart->legend()->setAlignment(Qt::AlignRight);

    Bounds bounds;
    bool addedAny = false;

    // --- Step 1: Add object curves (data + result) ---
    for (auto* obj : project->dataObjects())
        if (addObjectSeries(chart, obj, selectedObject, showControlPoints, bounds))
            addedAny = true;
    for (auto* obj : project->resultObjects())
        if (addObjectSeries(chart, obj, selectedObject, showControlPoints, bounds))
            addedAny = true;

    // --- Step 2: Draw normal arrows ---
    if (showNormals)
        addNormalArrows(chart, project, selectedObject, bounds, normalsCount, normalsLength);

    // --- Step 3: Create axes and attach all series ---
    if (addedAny)
        setupAxes(chart, bounds);

    // Re-enable signal emissions
    chart->blockSignals(false);
}

// ============================================================================
// addObjectSeries — add spline/scatter for a single object, returns true if
// any points were added
// ============================================================================
bool ChartWidget::addObjectSeries(QChart* chart, CustomObject* obj,
                                   CustomObject* selectedObject,
                                   bool showControlPoints, Bounds& bnd)
{
    if (!obj || !obj->isVisible()) return false;
    const auto& pts = obj->controlPoints();
    if (pts.isEmpty()) return false;

    bool pointObj = (toBaseType(obj->objectType()) == 0x001);
    bool isSelected = (obj == selectedObject);
    QColor color = isSelected ? QColor(255, 215, 0) : objectColor(obj);
    QString name = obj->name().isEmpty() ? QStringLiteral("Unnamed") : obj->name();

    if (pointObj) {
        // --- Single-point object: scatter marker ---
        auto* scatter = new QScatterSeries();
        scatter->setName(name);
        scatter->setColor(color);
        scatter->setBorderColor(color.darker(130));
        scatter->setMarkerSize(isSelected ? 14 : 10);
        scatter->setProperty("customObject", QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
        for (const auto& p : pts) {
            scatter->append(p.x(), p.y());
            bnd.minX = qMin(bnd.minX, p.x()); bnd.maxX = qMax(bnd.maxX, p.x());
            bnd.minY = qMin(bnd.minY, p.y()); bnd.maxY = qMax(bnd.maxY, p.y());
        }
        chart->addSeries(scatter);
        return true;
    }

    // --- Multi-point object: spline series ---
    auto* series = new QSplineSeries();
    series->setName(name);
    series->setPen(QPen(color, isSelected ? 3 : 2));
    series->setProperty("customObject", QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
    for (const auto& p : pts) {
        series->append(p.x(), p.y());
        bnd.minX = qMin(bnd.minX, p.x()); bnd.maxX = qMax(bnd.maxX, p.x());
        bnd.minY = qMin(bnd.minY, p.y()); bnd.maxY = qMax(bnd.maxY, p.y());
    }
    chart->addSeries(series);

    // --- Control-point scatter (hidden from legend) ---
    if (showControlPoints && pts.size() > 1) {
        auto* scatter = new QScatterSeries();
        scatter->setColor(color);
        scatter->setBorderColor(color.darker(130));
        scatter->setMarkerSize(isSelected ? 8 : 6);
        for (const auto& p : pts)
            scatter->append(p.x(), p.y());
        chart->addSeries(scatter);
        // Hide from legend
        auto markers = chart->legend()->markers(scatter);
        for (auto* marker : markers)
            marker->setVisible(false);
    }
    return true;
}

// ============================================================================
// addNormalArrows — draw normal vectors for all objects
// ============================================================================
void ChartWidget::addNormalArrows(QChart* chart, Project* project,
                                   CustomObject* selectedObject, Bounds& bnd,
                                   int normalsCount, double normalsLength)
{
    auto effectivePen = [selectedObject](CustomObject* obj) -> QColor {
        if (obj == selectedObject) return QColor(255, 215, 0);
        return objectColor(obj);
    };

    for (auto* obj : project->allObjects()) {
        if (!obj || !obj->isVisible()) continue;
        auto normals = obj->computeNormals(normalsCount, normalsLength);
        QColor color = effectivePen(obj);
        for (const auto& pair : normals) {
            auto* arrow = new QLineSeries();
            arrow->setPen(QPen(color, 1));
            arrow->append(pair.first.x(), pair.first.y());
            arrow->append(pair.second.x(), pair.second.y());
            chart->addSeries(arrow);
            // Hide from legend
            auto markers = chart->legend()->markers(arrow);
            for (auto* marker : markers)
                marker->setVisible(false);
            bnd.minX = qMin(bnd.minX, qMin(pair.first.x(), pair.second.x()));
            bnd.maxX = qMax(bnd.maxX, qMax(pair.first.x(), pair.second.x()));
            bnd.minY = qMin(bnd.minY, qMin(pair.first.y(), pair.second.y()));
            bnd.maxY = qMax(bnd.maxY, qMax(pair.first.y(), pair.second.y()));
        }
    }
}

// ============================================================================
// setupAxes — create value axes with padding and attach all series
// ============================================================================
void ChartWidget::setupAxes(QChart* chart, const Bounds& bnd)
{
    double dx = (bnd.maxX - bnd.minX) * 0.1;
    double dy = (bnd.maxY - bnd.minY) * 0.1;
    if (qFuzzyCompare(dx, 0.0)) dx = 1.0;
    if (qFuzzyCompare(dy, 0.0)) dy = 1.0;

    auto* axisX = new QValueAxis();
    axisX->setRange(bnd.minX - dx, bnd.maxX + dx);
    axisX->setLabelFormat(QStringLiteral("%.1f"));
    chart->addAxis(axisX, Qt::AlignBottom);

    auto* axisY = new QValueAxis();
    axisY->setRange(bnd.minY - dy, bnd.maxY + dy);
    axisY->setLabelFormat(QStringLiteral("%.1f"));
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto* series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
}

} // namespace ExpressDesigner
#endif // HAS_QT_CHARTS