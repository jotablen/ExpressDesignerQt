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

namespace ExpressDesigner {

void ChartWidget::populateChart(QChart* chart, Project* project,
                                 bool showControlPoints, bool showNormals,
                                 CustomObject* selectedObject)
{
    if (!chart || !project) return;
    chart->removeAllSeries();

    // Remove all axes too (removeAllSeries keeps existing axes)
    const auto axes = chart->axes();
    for (auto* axis : axes)
        chart->removeAxis(axis);

    bool addedAny = false;
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;

    auto addCurve = [&](CustomObject* obj, bool isSelected) {
        if (!obj || !obj->isVisible()) return;
        const auto& pts = obj->controlPoints();
        if (pts.isEmpty()) return;

        auto* series = new QLineSeries();
        series->setName(obj->name());
        if (isSelected) {
            QPen selectedPen(QColor(255, 215, 0)); // Gold/Yellow like Ovals Designer
            selectedPen.setWidth(3);
            series->setPen(selectedPen);
        }
        for (const auto& p : pts) {
            series->append(p.x(), p.y());
            minX = qMin(minX, p.x()); maxX = qMax(maxX, p.x());
            minY = qMin(minY, p.y()); maxY = qMax(maxY, p.y());
        }
        chart->addSeries(series);
        addedAny = true;

        // Show control points as scatter
        if (showControlPoints && pts.size() > 1) {
            auto* scatter = new QScatterSeries();
            scatter->setName(obj->name() + QStringLiteral(" (pts)"));
            scatter->setMarkerSize(6);
            for (const auto& p : pts)
                scatter->append(p.x(), p.y());
            chart->addSeries(scatter);
        }
    };

    for (auto* obj : project->dataObjects())
        addCurve(obj, obj == selectedObject);
    for (auto* obj : project->resultObjects())
        addCurve(obj, obj == selectedObject);

    // --- Draw normals if enabled ---
    if (showNormals) {
        for (auto* obj : project->allObjects()) {
            if (!obj || !obj->isVisible()) continue;
            auto normals = obj->computeNormals();
            for (const auto& pair : normals) {
                auto* arrow = new QLineSeries();
                arrow->append(pair.first.x(), pair.first.y());
                arrow->append(pair.second.x(), pair.second.y());
                QPen pen(obj->normalColor());
                pen.setWidth(1);
                arrow->setPen(pen);
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