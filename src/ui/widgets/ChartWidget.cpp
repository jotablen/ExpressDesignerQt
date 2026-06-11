#include "ChartWidget.h"
#ifdef HAS_QT_CHARTS
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <core/CustomObject.h>

namespace ExpressDesigner {

void ChartWidget::populateChart(QChart* chart, Project* project,
                                 bool showControlPoints, bool showNormals)
{
    if (!chart || !project) return;
    chart->removeAllSeries();

    auto addCurve = [&](CustomObject* obj) {
        if (!obj || !obj->isVisible()) return;
        auto* series = new QLineSeries();
        series->setName(obj->name());
        const auto& pts = obj->controlPoints();
        if (pts.size() >= 2 || obj->objectType() == ObjectType::Point) {
            for (const auto& p : pts)
                series->append(p.x(), p.y());
        }
        chart->addSeries(series);
    };

    for (auto* obj : project->dataObjects())
        addCurve(obj);
    for (auto* obj : project->resultObjects())
        addCurve(obj);
}

} // namespace ExpressDesigner
#endif // HAS_QT_CHARTS