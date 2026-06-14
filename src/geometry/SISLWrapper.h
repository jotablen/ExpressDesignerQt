#pragma once

#include <QVector>
#include <QPointF>
#include <QPair>

#ifdef USE_SISL
// Real SISL headers would be included here
// #include <sisl.h>
#endif

namespace ExpressDesigner {
namespace Geometry {

/**
 * @brief Lightweight wrapper for 2D point list operations.
 * Replaces OURList from GSNLib with standard Qt container.
 */
using PointList = QVector<QPointF>;

/**
 * @brief Normal representation: (origin, direction_end)
 */
using Normal = QPair<QPointF, QPointF>;

/**
 * @brief 2D curve representation using control points.
 * When SISL is available, wraps SISLCurve for NURBS operations.
 */
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

    QVector<QPointF> controlPoints() const;
    void setControlPoints(const QVector<QPointF>& points);

    static SISLCurve interpolate(const QVector<QPointF>& points, int order = 3);
    static SISLCurve approximate(const QVector<QPointF>& points, double tolerance = 0.01);

private:
    QVector<QPointF> m_points;
    int m_order = 3;
    bool m_open = true;
    bool m_valid = false;
};

/**
 * @brief 2D surface representation.
 */
class SISLSurface {
public:
    SISLSurface();
    ~SISLSurface();

    bool isValid() const;
    QPointF evaluate(double u, double v) const;

private:
    bool m_valid = false;
};

// Geometry operations
namespace Operations {
    QVector<QPointF> offsetCurve(const QVector<QPointF>& points, double distance);
    QVector<QPointF> intersectCurves(const QVector<QPointF>& a, const QVector<QPointF>& b);
    QVector<Normal> computeNormals2D(const QVector<QPointF>& points, int numRays, double rayLength, bool flipped);
    QVector<QPointF> discretizeArc(const QPointF& center, double radius, double startAngle, double endAngle, int numPoints);
}

} // namespace Geometry
} // namespace ExpressDesigner