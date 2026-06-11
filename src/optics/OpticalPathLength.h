#pragma once

#include <QVector>
#include <QPointF>

namespace ExpressDesigner {
namespace Optics {

double computeOPL(const QPointF& refPoint,
                  const QPointF& wf1Point, double idx1,
                  const QPointF& wf2Point, double idx2);

double computeOpticalPath(const QVector<QPointF>& path, const QVector<double>& indices);

} // namespace Optics
} // namespace ExpressDesigner