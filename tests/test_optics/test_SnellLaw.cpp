#include <gtest/gtest.h>
#include <optics/SnellLaw.h>
#include <QtMath>

using namespace ExpressDesigner;
using namespace ExpressDesigner::Optics;

// ======================================================================
// Snell's Law Test Suite — 20° incident, normal along Y
//
// n1 = 1.0, n2 = 1.5
// sin(θ1) = sin(20°) ≈ 0.342020
// n1·sin(θ1) = n2·sin(θ2)  →  sin(θ2) = sin(20°)/1.5 ≈ 0.228013
// θ2 = asin(0.228013) ≈ 13.180°
// cos(θ2) ≈ 0.97366
// ======================================================================

static constexpr double kTheta1 = 20.0;
static constexpr double kRad1  = qDegreesToRadians(kTheta1);
static constexpr double kSin1  = qSin(kRad1);   // ≈ 0.342020
static constexpr double kCos1  = qCos(kRad1);   // ≈ 0.939693
static constexpr double kSin2  = kSin1 / 1.5;   // ≈ 0.228013
static constexpr double kTheta2Deg = 13.180;    // expected deflected angle from normal
static constexpr double kCos2  = qCos(qAsin(kSin2));  // ≈ 0.97366

/// Helper: angle between two unit vectors in degrees
static double angleBetweenDeg(const QPointF& a, const QPointF& b)
{
    double dot = a.x() * b.x() + a.y() * b.y();
    dot = qBound(-1.0, dot, 1.0);
    return qRadiansToDegrees(qAcos(dot));
}

// ---------------------------------------------------------------------
// Test A: Normal = (0, +Y), incident 20° toward surface
//         Incident = (sin20, -cos20), Normal = (0, 1)
//         Expected output ~13.18° from -Y (downward)
TEST(SnellLawTest, Refraction_20deg_Normal_PosY)
{
    // Incident direction: 20° rotated from -Y, i.e. going DOWN and slightly RIGHT
    QPointF incident(kSin1, -kCos1);   // (0.3420, -0.9397)
    QPointF normal(0.0, 1.0);          // surface normal up
    bool tir = false;

    QPointF deflected = getDeflectedVector(incident, normal, 1.0, 1.5, tir);

    EXPECT_FALSE(tir) << "Should not be total internal reflection";

    // The deflected ray should point DOWN (negative Y — same side as incident)
    EXPECT_LT(deflected.y(), 0.0);

    // Angle between deflected ray and -Y axis ≈ θ2
    QPointF negY(0.0, -1.0);
    double angle = angleBetweenDeg(deflected, negY);
    EXPECT_NEAR(angle, kTheta2Deg, 0.5);

    // Magnitude should be 1 (unit vector)
    double mag = qSqrt(deflected.x() * deflected.x() + deflected.y() * deflected.y());
    EXPECT_NEAR(mag, 1.0, 1e-9);
}

// ---------------------------------------------------------------------
// Test B: Explicit check of the Ovals Designer vector formula
//         V2 = V1 - dot(V1,N)·N + sqrt(dot² - (n1²-n2²))·N
TEST(SnellLawTest, VectorFormula_Matches_getDeflected)
{
    QPointF incident(kSin1, -kCos1);
    QPointF normal(0.0, 1.0);
    bool tir = false;

    QPointF deflected = getDeflectedVector(incident, normal, 1.0, 1.5, tir);
    EXPECT_FALSE(tir);

    // Reproduce the Ovals Designer formula step by step
    QPointF dirIn = incident; // already unit
    QPointF N = normal;       // unit
    // getDeflectedVector flips N to face incident: dot(dirIn,N) = -kCos1 ≈ -0.94 < 0 → no flip
    double dotIn = dirIn.x() * N.x() + dirIn.y() * N.y();
    EXPECT_LT(dotIn, 0.0) << "Normal already faces incident, no flip needed";

    QPointF V1 = dirIn * 1.0; // vector of magnitude n1
    double dot = V1.x() * N.x() + V1.y() * N.y();
    double dot2 = dot * dot;
    double diff = 1.0 * 1.0 - 1.5 * 1.5; // n1² - n2² = 1 - 2.25 = -1.25

    double root = qSqrt(dot2 - diff);
    QPointF V2(V1.x() - dot * N.x() + root * N.x(),
               V1.y() - dot * N.y() + root * N.y());
    double mag = qSqrt(V2.x() * V2.x() + V2.y() * V2.y());
    V2 /= mag;                            // normalize to unit

    EXPECT_NEAR(deflected.x(), V2.x(), 1e-6);
    EXPECT_NEAR(deflected.y(), V2.y(), 1e-6);
}

// ---------------------------------------------------------------------
// Test C: Normal = (0, -Y), incident still passing through the surface
//         Incident points DOWN into medium with normal pointing DOWN
TEST(SnellLawTest, Refraction_20deg_Normal_NegY)
{
    // Normal is -Y; incident ray is 20° from +Y (since ray comes from above, going down)
    // Actually if normal = (0,-1) and ray is going down toward surface,
    // the ray direction is (sin20, -cos20) — same as before
    QPointF incident(kSin1, -kCos1);
    QPointF normal(0.0, -1.0);         // surface normal down
    bool tir = false;

    QPointF deflected = getDeflectedVector(incident, normal, 1.0, 1.5, tir);
    EXPECT_FALSE(tir);

    // Deflected should still go DOWN (negative Y)
    EXPECT_LT(deflected.y(), 0.0);

    // Angle between deflected and -Y
    QPointF negY(0.0, -1.0);
    double angle = angleBetweenDeg(deflected, negY);
    EXPECT_NEAR(angle, kTheta2Deg, 0.5);

    double mag = qSqrt(deflected.x() * deflected.x() + deflected.y() * deflected.y());
    EXPECT_NEAR(mag, 1.0, 1e-9);
}

// ---------------------------------------------------------------------
// Test D: Mirror (n1 == n2) → reflexion formula
//         Incident 20° from -Y, normal (0,1)
//         Reflection: output should be 20° from +Y (mirror angle)
TEST(SnellLawTest, Mirror_20deg_EqualIndices)
{
    QPointF incident(kSin1, -kCos1);  // 20° from -Y
    QPointF normal(0.0, 1.0);
    bool tir = false;

    QPointF deflected = getDeflectedVector(incident, normal, 1.0, 1.0, tir);
    EXPECT_TRUE(tir) << "Mirror returns TIR = true";

    // Reflected should go UP
    EXPECT_GT(deflected.y(), 0.0);

    // Angle from +Y should be 20°
    QPointF posY(0.0, 1.0);
    double angle = angleBetweenDeg(deflected, posY);
    EXPECT_NEAR(angle, kTheta1, 0.5);

    double mag = qSqrt(deflected.x() * deflected.x() + deflected.y() * deflected.y());
    EXPECT_NEAR(mag, 1.0, 1e-9);
}

// ---------------------------------------------------------------------
// Test E: Mirror with normal = (0, -Y)
TEST(SnellLawTest, Mirror_20deg_Normal_NegY)
{
    QPointF incident(kSin1, -kCos1);
    QPointF normal(0.0, -1.0);
    bool tir = false;

    QPointF deflected = getDeflectedVector(incident, normal, 1.0, 1.0, tir);
    EXPECT_TRUE(tir);

    // With normal (0,-1) and incident going down, dot > 0 → normal gets flipped to (0,1)
    // Then reflection should go up
    EXPECT_GT(deflected.y(), 0.0);

    QPointF posY(0.0, 1.0);
    double angle = angleBetweenDeg(deflected, posY);
    EXPECT_NEAR(angle, kTheta1, 0.5);

    double mag = qSqrt(deflected.x() * deflected.x() + deflected.y() * deflected.y());
    EXPECT_NEAR(mag, 1.0, 1e-9);
}

// ---------------------------------------------------------------------
// Test F: TIR — n1=1.5 → n2=1.0 at 50° (> 41.81° critical angle)
TEST(SnellLawTest, TIR_HighToLow)
{
    constexpr double tirDeg = 50.0;
    double s = qSin(qDegreesToRadians(tirDeg));
    double c = qCos(qDegreesToRadians(tirDeg));

    QPointF incident(s, -c);    // 50° from -Y
    QPointF normal(0.0, 1.0);  // surface normal up
    bool tir = false;

    QPointF deflected = getDeflectedVector(incident, normal, 1.5, 1.0, tir);
    EXPECT_TRUE(tir);

    // Reflected back up
    EXPECT_GT(deflected.y(), 0.0);

    double mag = qSqrt(deflected.x() * deflected.x() + deflected.y() * deflected.y());
    EXPECT_NEAR(mag, 1.0, 1e-9);
}

// ---------------------------------------------------------------------
// Test G: setDeflectionNormal recovers the surface normal
TEST(SnellLawTest, SetDeflectionNormal_TwentyDeg)
{
    // Given known incident & deflected, normal should be aligned with Y
    QPointF incident(kSin1, -kCos1);
    // Deflected = (sin(θ2), -cos(θ2))
    QPointF deflected(kSin2, -kCos2);
    QPointF surfacePt(0.0, 0.0);
    bool tir = false;

    QPointF N = setDeflectionNormal(surfacePt, incident, deflected, 1.0, 1.5, tir);
    EXPECT_FALSE(tir);

    // N should be ~(0, ±1)
    EXPECT_NEAR(qAbs(N.x()), 0.0, 0.01);
    EXPECT_NEAR(qAbs(N.y()), 1.0, 0.01);

    double mag = qSqrt(N.x() * N.x() + N.y() * N.y());
    EXPECT_NEAR(mag, 1.0, 1e-9);
}