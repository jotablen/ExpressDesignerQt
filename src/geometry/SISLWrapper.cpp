#include "SISLWrapper.h"
#include "VectorUtils.h"
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
    return flipped ? -n : n;
}

QPointF SISLCurve::normalAt(const QPointF& p) const
{
    double t;
    closestPoint(p, &t);
    return normal(t);
}

QPair<QPointF, QPointF> SISLCurve::pointAndDerivative(double t) const
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
    return {QPointF(result[0], result[1]), QPointF(result[3], result[4])};
}

QPair<QPointF, QPointF> SISLCurve::pointAndNormal(double t, bool flipped) const
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
    double len = qSqrt(result[3] * result[3] + result[4] * result[4]);
    QPointF n;
    if (len < 1e-12)
        n = flipped ? QPointF(0, -1) : QPointF(0, 1);
    else
        n = QPointF(-result[4] / len, result[3] / len);
    if (flipped) n = -n;
    return {QPointF(result[0], result[1]), n};
}

// --- Closest point N (ODs Find_CP_Repaired) ---
bool SISLCurve::closestPointN(const QPointF& ref, QPointF& outPt, QPointF& outNormal, bool flipped) const
{
    double t;
    QPointF cp = closestPoint(ref, &t);
    QPointF n = normal(t, flipped);
    double cr = cross(n, cp - ref);
    if (qAbs(cr) > 1e-10) {
        double proj = dot(n, cp - ref);
        outPt = ref + n * proj;
        outNormal = n;
        return true;
    }
    outPt = cp;
    outNormal = n;
    return true;
}

// --- Discretized closest-point fallback (fast polyline search) ---
QPointF SISLCurve::closestPointDiscretized(const QPointF& p, double* outT) const
{
    auto dense = evaluateAll(200);
    double bestDistSq = std::numeric_limits<double>::max();
    QPointF bestPt = dense.isEmpty() ? QPointF() : dense[0];
    double bestT = 0.0;
    for (int i = 0; i < dense.size() - 1; ++i) {
        QPointF a = dense[i], b = dense[i + 1];
        QPointF ab = b - a;
        double abLenSq = lengthSq(ab);
        if (abLenSq < 1e-18) {
            double d2 = distSq(a, p);
            if (d2 < bestDistSq) { bestDistSq = d2; bestPt = a; bestT = (double)i / (dense.size() - 1); }
            continue;
        }
        double segT = dot(p - a, ab) / abLenSq;
        segT = qBound(0.0, segT, 1.0);
        QPointF proj = a + ab * segT;
        double d2 = distSq(proj, p);
        if (d2 < bestDistSq) { bestDistSq = d2; bestPt = proj; bestT = (i + segT) / (dense.size() - 1); }
    }
    if (outT) *outT = qBound(0.0, bestT, 1.0);
    return bestPt;
}

// --- Closest point (ODs SISLDrink: s1953 — native SISL, no discretization) ---
QPointF SISLCurve::closestPoint(const QPointF& p, double* outT) const
{
    auto* c = static_cast< ::SISLCurve*>(m_curve);
    if (!c || m_points.size() < 2) return m_points.isEmpty() ? QPointF() : m_points[0];

    double point[3] = { p.x(), p.y(), 0.0 };
    constexpr double epsco = 1e-12;
    constexpr double epsge = 1e-12;
    int numintpt = 0;
    double* intpar = nullptr;
    int numintcu = 0;
    ::SISLIntcurve** intcurve = nullptr;
    int error = 0;

    s1953(c, point, 3, epsco, epsge, &numintpt, &intpar,
          &numintcu, &intcurve, &error);

    if (numintcu)
        freeIntcrvlist(intcurve, numintcu);

    if (numintpt <= 0 || !intpar) {
        if (intpar) delete[] intpar;
        return closestPointDiscretized(p, outT);
    }

    double tVal = intpar[0];
    delete[] intpar;

    auto [pt, _] = pointAndDerivative(tVal);
    if (outT) *outT = qBound(0.0, tVal, 1.0);
    return pt;
}

// --- Plane intersection (ODs SISLDrink: s1850 + ODs normal computation) ---
double SISLCurve::planeIntersection(const QPointF& planePoint, const QPointF& planeNormal,
                                    QPointF* outHitPt, QPointF* outNormal,
                                    double* outT) const
{
    auto* c = static_cast< ::SISLCurve*>(m_curve);
    if (!c) return -1.0;

    double point[3] = { planePoint.x(), planePoint.y(), 0.0 };
    double normalPlano[3] = { planeNormal.x(), planeNormal.y(), 0.0 };
    double epsco = 1e-8, epsge = 1e-8;
    int numintpt = 0;
    double* ListaParametros = nullptr;
    int numintcu = 0;
    ::SISLIntcurve** intcurve = nullptr;
    int SISLError_val = 0;

    s1850(c, point, normalPlano, 3, epsco, epsge,
          &numintpt, &ListaParametros,
          &numintcu, &intcurve, &SISLError_val);

    if (numintcu)
        freeIntcrvlist(intcurve, numintcu);

    if (numintpt <= 0 || !ListaParametros)
        return -1.0;

    auto [hitPt, tangent] = pointAndDerivative(ListaParametros[0]);
    QPointF curveNormal = curveNormal2D(tangent);

    delete[] ListaParametros;

    if (outHitPt) *outHitPt = hitPt;
    if (outNormal) *outNormal = curveNormal;
    if (outT) *outT = 0.0;

    return dist(hitPt, planePoint);
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