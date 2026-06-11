#pragma once

#include <QPointF>
#include <QPair>

namespace ExpressDesigner {
namespace Optics {

/**
 * @brief Snell's Law calculations for optical ray deflection.
 */
struct SnellResult {
    QPointF deflectedDirection;
    bool totalInternalReflection = false;
    double transmittedAngle = 0.0;
};

SnellResult deflectRay(const QPointF& incidentDirection,
                       const QPointF& surfaceNormal,
                       double n1, double n2);

QPointF getDeflectedVector(const QPointF& vecIn, const QPointF& surfacePoint,
                           double indexIn, double indexOut, bool& tir);

/**
 * @brief Compute deflection normal at a surface point.
 * @param surfacePoint Point on the surface
 * @param vecIn Incident vector
 * @param vecOut Output vector direction
 * @param indexIn Input refractive index
 * @param indexOut Output refractive index
 * @param tir Output - true if total internal reflection occurs
 * @return Normal vector at the surface point
 */
QPointF setDeflectionNormal(const QPointF& surfacePoint,
                            const QPointF& vecIn,
                            const QPointF& vecOut,
                            double indexIn, double indexOut,
                            bool& tir);

} // namespace Optics
} // namespace ExpressDesigner