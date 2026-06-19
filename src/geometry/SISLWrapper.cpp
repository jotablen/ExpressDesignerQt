#include "SISLWrapper.h"
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstring>

#include <sisl.h>

namespace ExpressDesigner {
namespace Geometry {

// ============================================================================
// Helper: build a cubic B-spline from control points
// ============================================================================
static ::SISLCurve* buildSpline(const QVector<QPointF>& points)
{
    int n = points.size();
    if (n < 2) return nullptr;

    int order = qMin(4, n);
    int ik = order;
    int in = n;

    int nKnots = in + ik;
    double* et = new double[nKnots];
    for (int i = 0; i < ik; ++i)
        et[i] = 0.0;
    for (int i = ik; i < in; ++i)
        et[i] = static_cast<double>(i - ik + 1) / (in - ik + 1);
    for (int i = in; i < nKnots; ++i)
        et[i] = 1.0;

    int idim = 3;
    double* ecoef = new double[n * idim];
    for (int i = 0; i < n; ++i) {
        ecoef[i * idim + 0] = points[i].x();
        ecoef[i * idim + 1] = points[i].y();
        ecoef[i * idim + 2] = 0.0;
    }

    int icopy = 0;
    ::SISLCurve* curve = newCurve(in, ik, et, ecoef, 1, idim, icopy);
    if (!curve) {
        delete[] et;
        delete[] ecoef;
    }
    return curve;
}

// ============================================================================
// SISLCurve implementation
// ============================================================================
SISLCurve::SISLCurve() = default;

SISLCurve::SISLCurve(const QVector<QPointF>& controlPoints, int order, bool open)
    : m_points(controlPoints), m_order(order), m_open(open)
{
    m_valid = !controlPoints.isEmpty() && order >= 2;
    if (m_valid && controlPoints.size() >= 2) {
        m_curve = buildSpline(controlPoints);
        if (!m_curve) m_valid = false;
    }
}

SISLCurve::~SISLCurve()
{
    if (m_curve) {
        freeCurve(static_cast< ::SISLCurve*>(m_curve));
        m_curve = nullptr;
    }
}

SISLCurve::SISLCurve(SISLCurve&& other) noexcept
    : m_curve(other.m_curve), m_points(std::move(other.m_points))
    , m_order(other.m_order), m_open(other.m_open), m_valid(other.m_valid)
{
    other.m_curve = nullptr;
}

SISLCurve& SISLCurve::operator=(SISLCurve&& other) noexcept
{
    if (this != &other) {
        if (m_curve) freeCurve(static_cast< ::SISLCurve*>(m_curve));
        m_curve = other.m_curve;
        m_points = std::move(other.m_points);
        m_order = other.m_order;
        m_open = other.m_open;
        m_valid = other.m_valid;
        other.m_curve = nullptr;
    }
    return *this;
}

bool SISLCurve::isValid() const { return m_valid && m_curve != nullptr; }
int SISLCurve::order() const { return m_order; }
bool SISLCurve::isOpen() const { return m_open; }

QVector<QPointF> SISLCurve::controlPoints() const { return m_points; }

void SISLCurve::setControlPoints(const QVector<QPointF>& points)
{
    if (m_curve) { freeCurve(static_cast< ::SISLCurve*>(m_curve)); m_curve = nullptr; }
    m_points = points;
    m_valid = !points.isEmpty();
    if (m_valid && points.size() >= 2)
        m_curve = buildSpline(points);
}

// --- SISL NURBS evaluation ---
QPointF SISLCurve::evaluate(double t) const
{
    if (!m_curve || m_points.isEmpty()) return {};
    auto* c = static_cast< ::SISLCurve*>(m_curve);
    double param = qBound(0.0, t, 1.0);
    double* et = c->et;
    if (!et) return {};
    int ik = c->ik;
    int in_val = c->in;
    double tmin = et[ik - 1];
    double tmax = et[in_val];
    double u = tmin + param * (tmax - tmin);
    int iKnot = 0;
    double result[3] = {};
    int status = 0;
    s1227(c, 0, u, &iKnot, result, &status);
    if (status < 0) return {};
    return QPointF(result[0], result[1]);
}

QVector<QPointF> SISLCurve::evaluateAll(int numPoints) const
{
    QVector<QPointF> result;
    result.reserve(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        double t = numPoints > 1 ? static_cast<double>(i) / (numPoints - 1) : 0.0;
        result.append(evaluate(t));
    }
    return result;
}

QPointF SISLCurve::derivative(double t) const
{
    if (!m_curve || m_points.size() < 2) return {};
    auto* c = static_cast< ::SISLCurve*>(m_curve);
    double param = qBound(0.0, t, 1.0);
    double* et = c->et;
    if (!et) return {};
    int ik = c->ik;
    int in_val = c->in;
    double tmin = et[ik - 1];
    double tmax = et[in_val];
    double u = tmin + param * (tmax - tmin);
    int iKnot = 0;
    double result[6] = {};
    int status = 0;
    s1227(c, 1, u, &iKnot, result, &status);
    if (status < 0) return {};
    return QPointF(result[3], result[4]);
}

QPointF SISLCurve::normal(double t, bool flipped) const
{
    QPointF d = derivative(t);
    double len = qSqrt(d.x() * d.x() + d.y() * d.y());
    if (len < 1e-12) return flipped ? QPointF(0, -1) : QPointF(0, 1);
    QPointF n(-d.y() / len, d.x() / len);
    return flipped ? QPointF(-n.x(), -n.y()) : n;
}

QPointF SISLCurve::normalAt(const QPointF& p) const
{
    double t;
    closestPoint(p, &t);
    return normal(t);
}

// --- Closest point (discretized search on NURBS) ---
QPointF SISLCurve::closestPoint(const QPointF& p, double* outT) const
{
    if (!m_curve || m_points.size() < 2) return m_points.isEmpty() ? QPointF() : m_points[0];
    auto dense = evaluateAll(200);
    double bestDistSq = std::numeric_limits<double>::max();
    QPointF bestPt;
    double bestT = 0.0;
    int n = dense.size();
    for (int i = 0; i < n - 1; ++i) {
        QPointF a = dense[i], b = dense[i + 1];
        QPointF ab = b - a;
        double abLenSq = ab.x() * ab.x() + ab.y() * ab.y();
        if (abLenSq < 1e-18) {
            double d2 = (p.x() - a.x()) * (p.x() - a.x()) + (p.y() - a.y()) * (p.y() - a.y());
            if (d2 < bestDistSq) { bestDistSq = d2; bestPt = a; bestT = (double)i / (n - 1); }
            continue;
        }
        double segT = ((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / abLenSq;
        segT = qBound(0.0, segT, 1.0);
        QPointF proj = a + ab * segT;
        double d2 = (p.x() - proj.x()) * (p.x() - proj.x()) + (p.y() - proj.y()) * (p.y() - proj.y());
        if (d2 < bestDistSq) { bestDistSq = d2; bestPt = proj; bestT = (i + segT) / (n - 1); }
    }
    if (outT) *outT = qBound(0.0, bestT, 1.0);
    return bestPt;
}

// --- Plane intersection (ODs SISLDrink: s1850) ---
double SISLCurve::planeIntersection(const QPointF& planePoint, const QPointF& planeNormal,
                                    QPointF* outHitPt, double* outT) const
{
    auto* c = static_cast< ::SISLCurve*>(m_curve);
    if (!c) return -1.0;

    double point[3] = { planePoint.x(), planePoint.y(), 0.0 };
    double normal[3] = { planeNormal.x(), planeNormal.y(), 0.0 };
    double epsco = 1e-8, epsge = 1e-8;
    int numintpt = 0;
    double* ListaParametros = nullptr;
    int numintcu = 0;
    ::SISLIntcurve** intcurve = nullptr;
    int SISLError_val = 0;

    s1850(c, point, normal, 3, epsco, epsge,
          &numintpt, &ListaParametros,
          &numintcu, &intcurve, &SISLError_val);

    if (numintcu)
        freeIntcrvlist(intcurve, numintcu);

    if (numintpt <= 0 || !ListaParametros)
        return -1.0;

    int iKnot = 0;
    double PontoTangente[6] = {};
    s1227(c, 1, ListaParametros[0], &iKnot, PontoTangente, &SISLError_val);

    QPointF hitPt(PontoTangente[0], PontoTangente[1]);
    delete[] ListaParametros;

    if (outHitPt) *outHitPt = hitPt;
    if (outT) *outT = 0.0;

    double dx = hitPt.x() - planePoint.x();
    double dy = hitPt.y() - planePoint.y();
    return qSqrt(dx * dx + dy * dy);
}

// --- Static factory ---
SISLCurve SISLCurve::interpolate(const QVector<QPointF>& points, int order) { return SISLCurve(points, order); }
SISLCurve SISLCurve::approximate(const QVector<QPointF>& points, double) { return SISLCurve(points, 3); }

// ============================================================================
// SISLSurface (stub)
// ============================================================================
SISLSurface::SISLSurface() = default;
SISLSurface::~SISLSurface() = default;
bool SISLSurface::isValid() const { return m_valid; }
QPointF SISLSurface::evaluate(double, double) const { return {}; }

// ============================================================================
// Operations
// ============================================================================
namespace Operations {

QVector<QPointF> offsetCurve(const QVector<QPointF>& points, double distance)
{
    if (points.size() < 2) return points;
    QVector<QPointF> result;
    result.reserve(points.size());
    SISLCurve curve(points, 3, true);
    int n = points.size();
    for (int i = 0; i < n; ++i) {
        double t = n > 1 ? static_cast<double>(i) / (n - 1) : 0.0;
        QPointF nrm = curve.normal(t);
        result.append(points[i] + nrm * distance);
    }
    return result;
}

QVector<QPointF> intersectCurves(const QVector<QPointF>& a, const QVector<QPointF>& b)
{
    Q_UNUSED(a); Q_UNUSED(b);
    return {};
}

QVector<Normal> computeNormals2D(const QVector<QPointF>& points, int numRays, double rayLength, bool flipped)
{
    QVector<Normal> result;
    result.reserve(numRays);
    SISLCurve curve(points, 3, true);
    for (int i = 0; i < numRays; ++i) {
        double t = numRays > 1 ? static_cast<double>(i) / (numRays - 1) : 0.0;
        QPointF origin = curve.evaluate(t);
        QPointF nrm = curve.normal(t, flipped);
        result.append({origin, origin + nrm * rayLength});
    }
    return result;
}

QVector<QPointF> discretizeArc(const QPointF& center, double radius, double startAngle, double endAngle, int numPoints)
{
    QVector<QPointF> result;
    result.reserve(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        double t = numPoints > 1 ? static_cast<double>(i) / (numPoints - 1) : 0.0;
        double angle = startAngle + (endAngle - startAngle) * t;
        result.append(QPointF(center.x() + radius * qCos(angle), center.y() + radius * qSin(angle)));
    }
    return result;
}

} // namespace Operations
} // namespace Geometry
} // namespace ExpressDesigner