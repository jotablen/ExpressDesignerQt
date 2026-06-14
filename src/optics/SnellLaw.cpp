#include "SnellLaw.h"
#include <QtMath>

namespace ExpressDesigner {
namespace Optics {

// --- Reflection (n1 == n2) — Ovals Designer reflexion ---
// V_out = V_in - 2 * dot(V_in, N) * N
static QPointF reflexion(const QPointF& V1, const QPointF& N)
{
    double dot = V1.x() * N.x() + V1.y() * N.y();
    double twoDot = 2.0 * dot;
    return QPointF(V1.x() - twoDot * N.x(), V1.y() - twoDot * N.y());
}

// --- Refraction (n1 != n2) — Ovals Designer refraccion ---
// V2 = V1 - dot(V1,N)*N + sqrt(dot(V1,N)^2 - (n1^2 - n2^2)) * N
// TIR: n1^2 - n2^2 > dot(V1,N)^2
static QPointF refraccion(const QPointF& V1, const QPointF& N,
                           double n1, double n2, bool& tir)
{
    double f_V1_N = V1.x() * N.x() + V1.y() * N.y();
    double f_V1_N_2 = f_V1_N * f_V1_N;
    double f_n12_n22 = n1 * n1 - n2 * n2;

    tir = (f_n12_n22 > f_V1_N_2);

    if (!tir) {
        double disc = qSqrt(f_V1_N_2 - f_n12_n22);
        QPointF V2(V1.x() - f_V1_N * N.x() + disc * N.x(),
                    V1.y() - f_V1_N * N.y() + disc * N.y());
        // V2 has magnitude n2 — normalize to unit
        double len = qSqrt(V2.x() * V2.x() + V2.y() * V2.y());
        if (len > 1e-12) V2 /= len;
        return V2;
    }

    // TIR → fall back to reflection
    double twoDot = 2.0 * f_V1_N;
    QPointF V2(V1.x() - twoDot * N.x(), V1.y() - twoDot * N.y());
    double len = qSqrt(V2.x() * V2.x() + V2.y() * V2.y());
    if (len > 1e-12) V2 /= len;
    return V2;
}

SnellResult deflectRay(const QPointF& incidentDirection,
                       const QPointF& surfaceNormal,
                       double n1, double n2)
{
    SnellResult result;
    double lenIn = qSqrt(incidentDirection.x() * incidentDirection.x() +
                          incidentDirection.y() * incidentDirection.y());
    double lenN = qSqrt(surfaceNormal.x() * surfaceNormal.x() +
                         surfaceNormal.y() * surfaceNormal.y());
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
        // Equal indices → mirror (reflection)
        result.totalInternalReflection = true;
        result.deflectedDirection = reflexion(V1, normal);
    } else {
        // Different indices → refraction
        bool tir = false;
        result.deflectedDirection = refraccion(V1, normal, n1, n2, tir);
        result.totalInternalReflection = tir;
    }
    return result;
}

QPointF getDeflectedVector(const QPointF& vecIn, const QPointF& surfaceNormal,
                           double indexIn, double indexOut, bool& tir)
{
    double lenIn = qSqrt(vecIn.x() * vecIn.x() + vecIn.y() * vecIn.y());
    double lenN = qSqrt(surfaceNormal.x() * surfaceNormal.x() +
                         surfaceNormal.y() * surfaceNormal.y());
    if (lenIn < 1e-12 || lenN < 1e-12) {
        tir = false;
        return vecIn;
    }

    QPointF dirIn = vecIn / lenIn;
    QPointF normal = surfaceNormal / lenN;

    // Ovals Designer convention: normal should point TOWARD the incident ray (dot > 0)
    double dotIn = dirIn.x() * normal.x() + dirIn.y() * normal.y();
    if (dotIn < 0) normal = QPointF(-normal.x(), -normal.y());

    QPointF V1 = dirIn * indexIn;

    if (qAbs(indexIn - indexOut) < 1e-12) {
        // Mirror
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
    double lenIn = qSqrt(vecIn.x() * vecIn.x() + vecIn.y() * vecIn.y());
    double lenOut = qSqrt(vecOut.x() * vecOut.x() + vecOut.y() * vecOut.y());
    if (lenIn < 1e-12 || lenOut < 1e-12) { tir = false; return QPointF(0, 1); }

    QPointF dirIn = vecIn / lenIn;
    QPointF dirOut = vecOut / lenOut;

    // N = r_out - i_in * (n1/n2)   [Ovals Designer normalrr/normalrx]
    double ratio = indexIn / indexOut;
    QPointF normal(dirOut.x() - dirIn.x() * ratio,
                    dirOut.y() - dirIn.y() * ratio);

    // TIR check: acos(dot(in,out)) > π/2 - θ_crit
    double dot = dirIn.x() * dirOut.x() + dirIn.y() * dirOut.y();
    dot = qBound(-1.0, dot, 1.0);
    double angle = qAcos(dot);
    double thetaCrit = qAsin(qMin(indexIn, indexOut) / qMax(indexIn, indexOut));
    tir = (angle > (M_PI_2 - thetaCrit));

    double nLen = qSqrt(normal.x() * normal.x() + normal.y() * normal.y());
    if (nLen > 1e-12)
        normal /= nLen;
    return normal;
}

} // namespace Optics
} // namespace ExpressDesigner