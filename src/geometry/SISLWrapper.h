#pragma once

#include <QVector>
#include <QPointF>
#include <QPair>

namespace ExpressDesigner {
namespace Geometry {

using PointList = QVector<QPointF>;
using Normal = QPair<QPointF, QPointF>;

class SISLCurve {
public:
    SISLCurve();
    SISLCurve(const QVector<QPointF>& controlPoints, int order = 3, bool open = true);
    ~SISLCurve();

    SISLCurve(const SISLCurve&) = delete;
    SISLCurve& operator=(const SISLCurve&) = delete;
    SISLCurve(SISLCurve&&) noexcept;
    SISLCurve& operator=(SISLCurve&&) noexcept;

    bool isValid() const;
    int order() const;
    bool isOpen() const;

    QPointF evaluate(double t) const;
    QVector<QPointF> evaluateAll(int numPoints) const;
    QPointF derivative(double t) const;
    QPointF normal(double t, bool flipped = false) const;

    QPointF closestPoint(const QPointF& p, double* outT = nullptr) const;

    /// Discretized closest-point fallback (fast polyline search).
    QPointF closestPointDiscretized(const QPointF& p, double* outT = nullptr) const;

    /// Find the closest point on the curve whose normal (optional flip) passes through ref.
    /// Returns the repaired point and its normal in one call.
    /// Returns false if no such point exists (cross product tolerance).
    bool closestPointN(const QPointF& ref, QPointF& outPt, QPointF& outNormal,
                       bool flipped = false) const;

    QPair<QPointF, QPointF> pointAndDerivative(double t) const;
    QPair<QPointF, QPointF> pointAndNormal(double t, bool flipped = false) const;
    double   planeIntersection(const QPointF& planePoint, const QPointF& planeNormal,
                               QPointF* outHitPt = nullptr, QPointF* outNormal = nullptr,
                               double* outT = nullptr) const;
    QPointF normalAt(const QPointF& p) const;

    QVector<QPointF> controlPoints() const;
    void setControlPoints(const QVector<QPointF>& points);

    static SISLCurve interpolate(const QVector<QPointF>& points, int order = 3);
    static SISLCurve approximate(const QVector<QPointF>& points, double tolerance = 0.01);

private:
    // SISL native curve (void* to avoid exposing sisl.h in this header)
    void* m_curve = nullptr;

    // Backup polyline for when SISL is not available
    QVector<QPointF> m_points;
    int m_order = 3;
    bool m_open = true;
    bool m_valid = false;
};

class SISLSurface {
public:
    SISLSurface();
    ~SISLSurface();
    bool isValid() const;
    QPointF evaluate(double u, double v) const;
private:
    bool m_valid = false;
};

namespace Operations {
    QVector<QPointF> offsetCurve(const QVector<QPointF>& points, double distance);
    QVector<QPointF> intersectCurves(const QVector<QPointF>& a, const QVector<QPointF>& b);
    QVector<Normal> computeNormals2D(const QVector<QPointF>& points, int numRays, double rayLength, bool flipped);
    QVector<QPointF> discretizeArc(const QPointF& center, double radius, double startAngle, double endAngle, int numPoints);
}

} // namespace Geometry
} // namespace ExpressDesigner