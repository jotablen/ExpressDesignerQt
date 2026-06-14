#include "SnellLaw.h"
#include <QtMath>

namespace ExpressDesigner {
namespace Optics {

SnellResult deflectRay(const QPointF& incidentDirection,
                       const QPointF& surfaceNormal,
                       double n1, double n2)
{
    SnellResult result;
    double dot = incidentDirection.x() * surfaceNormal.x() +
                 incidentDirection.y() * surfaceNormal.y();
    double sinTheta1 = qSqrt(1.0 - dot * dot);
    double ratio = n1 / n2;
    double sinTheta2 = ratio * sinTheta1;

    if (sinTheta2 > 1.0) {
        result.totalInternalReflection = true;
        result.deflectedDirection = QPointF(
            incidentDirection.x() - 2.0 * dot * surfaceNormal.x(),
            incidentDirection.y() - 2.0 * dot * surfaceNormal.y());
    } else {
        result.totalInternalReflection = false;
        double cosTheta2 = qSqrt(1.0 - sinTheta2 * sinTheta2);
        QPointF tangentialComponent = incidentDirection - dot * surfaceNormal;
        double magTan = qSqrt(tangentialComponent.x() * tangentialComponent.x() +
                               tangentialComponent.y() * tangentialComponent.y());
        if (magTan > 1e-12) {
            QPointF tanDir = tangentialComponent / magTan;
            result.deflectedDirection = ratio * tanDir * sinTheta2 / sinTheta1 + surfaceNormal * cosTheta2;
        } else {
            result.deflectedDirection = surfaceNormal;
        }
        result.transmittedAngle = qAsin(sinTheta2);
    }
    return result;
}

QPointF getDeflectedVector(const QPointF& vecIn, const QPointF& surfaceNormal,
                           double indexIn, double indexOut, bool& tir)
{
    double lenIn = qSqrt(vecIn.x() * vecIn.x() + vecIn.y() * vecIn.y());
    if (lenIn < 1e-12) { tir = false; return vecIn; }
    double lenN = qSqrt(surfaceNormal.x() * surfaceNormal.x() + surfaceNormal.y() * surfaceNormal.y());
    if (lenN < 1e-12) { tir = false; return vecIn; }

    QPointF dirIn = vecIn / lenIn;
    QPointF normal = surfaceNormal / lenN;

    // Ensure normal points toward incident ray (negative dot product = facing)
    double dotIn = dirIn.x() * normal.x() + dirIn.y() * normal.y();
    if (dotIn > 0) normal = QPointF(-normal.x(), -normal.y());

    // Refraction or reflection using Snell's law with the surface normal
    auto result = deflectRay(dirIn, normal, indexIn, indexOut);
    tir = result.totalInternalReflection;
    return result.deflectedDirection;
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
    double ratio = indexIn / indexOut;
    tir = (ratio > 1.0) && (qAbs(dirIn.x() * dirOut.y() - dirIn.y() * dirOut.x()) > 1.0);
    QPointF normal = QPointF(indexOut * dirOut.x() - indexIn * dirIn.x(),
                              indexOut * dirOut.y() - indexIn * dirIn.y());
    // Normalize the resulting normal vector
    double nLen = qSqrt(normal.x() * normal.x() + normal.y() * normal.y());
    if (nLen > 1e-12)
        normal /= nLen;
    return normal;
}

} // namespace Optics
} // namespace ExpressDesigner