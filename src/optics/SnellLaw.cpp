#include "SnellLaw.h"
#include <geometry/VectorUtils.h>
#include <QtMath>

namespace ExpressDesigner {
namespace Optics {

using namespace Geometry;

/// Reflection (n1 == n2) — Ovals Designer reflexion
/// V_out = V_in - 2 * dot(V_in, N) * N
static QPointF reflexion(const QPointF& V1, const QPointF& N)
{
    return V1 - 2.0 * dot(V1, N) * N;
}

/// Refraction (n1 != n2) — Ovals Designer refraccion
/// V2 = V1 - dot(V1,N)*N + sqrt(dot(V1,N)^2 - (n1^2 - n2^2)) * N
/// TIR: n1^2 - n2^2 > dot(V1,N)^2
static QPointF refraccion(const QPointF& V1, const QPointF& N,
                           double n1, double n2, bool& tir)
{
    double f = dot(V1, N);
    double f2 = f * f;
    double d = n1 * n1 - n2 * n2;

    tir = (d > f2);

    if (!tir) {
        double disc = qSqrt(f2 - d);
        QPointF V2 = V1 - f * N + disc * N;
        double len = length(V2);
        return len > 1e-12 ? V2 / len : V2;
    }

    // TIR → fall back to reflection
    QPointF V2 = V1 - 2.0 * f * N;
    double len = length(V2);
    return len > 1e-12 ? V2 / len : V2;
}

SnellResult deflectRay(const QPointF& incidentDirection,
                       const QPointF& surfaceNormal,
                       double n1, double n2)
{
    SnellResult result;
    double lenIn = length(incidentDirection);
    double lenN = length(surfaceNormal);
    if (lenIn < 1e-12 || lenN < 1e-12) {
        result.totalInternalReflection = false;
        result.deflectedDirection = incidentDirection;
        return result;
    }

    QPointF dirIn = incidentDirection / lenIn;
    QPointF normal = surfaceNormal / lenN;

    // Scale incident to magnitude n1 (Ovals Designer convention)
    QPointF V1 = dirIn * n1;

    if (qAbs(n1 - n2) < 1e-12) {
        result.totalInternalReflection = true;
        result.deflectedDirection = reflexion(V1, normal);
    } else {
        bool tir = false;
        result.deflectedDirection = refraccion(V1, normal, n1, n2, tir);
        result.totalInternalReflection = tir;
    }
    return result;
}

QPointF getDeflectedVector(const QPointF& vecIn, const QPointF& surfaceNormal,
                           double indexIn, double indexOut, bool& tir)
{
    double lenIn = length(vecIn);
    double lenN = length(surfaceNormal);
    if (lenIn < 1e-12 || lenN < 1e-12) {
        tir = false;
        return vecIn;
    }

    QPointF dirIn = vecIn / lenIn;
    QPointF normal = surfaceNormal / lenN;

    // Ovals Designer convention: normal should point TOWARD the incident ray
    if (dot(dirIn, normal) < 0) normal = -normal;

    QPointF V1 = dirIn * indexIn;

    if (qAbs(indexIn - indexOut) < 1e-12) {
        tir = true;
        return reflexion(V1, normal);
    }

    return refraccion(V1, normal, indexIn, indexOut, tir);
}

QPointF setDeflectionNormal(const QPointF& surfacePoint,
                            const QPointF& vecIn,
                            const QPointF& vecOut,
                            double indexIn, double indexOut,
                            bool& tir)
{
    Q_UNUSED(surfacePoint);
    double lenIn = length(vecIn);
    double lenOut = length(vecOut);
    if (lenIn < 1e-12 || lenOut < 1e-12) { tir = false; return QPointF(0, 1); }

    QPointF dirIn = vecIn / lenIn;
    QPointF dirOut = vecOut / lenOut;

    // N = r_out - i_in * (n1/n2)
    double ratio = indexIn / indexOut;
    QPointF normal = dirOut - dirIn * ratio;

    // TIR check: acos(dot(in,out)) > π/2 - θ_crit
    double d = dot(dirIn, dirOut);
    d = qBound(-1.0, d, 1.0);
    double angle = qAcos(d);
    double thetaCrit = qAsin(qMin(indexIn, indexOut) / qMax(indexIn, indexOut));
    tir = (angle > (M_PI_2 - thetaCrit));

    double nLen = length(normal);
    if (nLen > 1e-12)
        normal = normal / nLen;
    return normal;
}

} // namespace Optics
} // namespace ExpressDesigner