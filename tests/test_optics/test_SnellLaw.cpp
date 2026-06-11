#include <gtest/gtest.h>
#include <optics/SnellLaw.h>

using namespace ExpressDesigner;
using namespace ExpressDesigner::Optics;

TEST(SnellLawTest, NoRefraction)
{
    QPointF incident(1.0, 0.0);
    QPointF normal(0.0, -1.0);
    bool tir = false;

    QPointF result = getDeflectedVector(incident, normal, 1.0, 1.0, tir);
    EXPECT_FALSE(tir);
}

TEST(SnellLawTest, NormalIncidence)
{
    QPointF incident(0.0, -1.0);
    QPointF normal(0.0, -1.0);
    bool tir = false;

    QPointF result = getDeflectedVector(incident, normal, 1.0, 1.5, tir);
    EXPECT_FALSE(tir);
}

TEST(SnellLawTest, DeflectRay)
{
    QPointF incident(0.7071, -0.7071); // 45 degrees
    QPointF normal(0.0, -1.0);

    SnellResult sr = deflectRay(incident, normal, 1.0, 1.5);
    EXPECT_FALSE(sr.totalInternalReflection);
    double length = std::sqrt(sr.deflectedDirection.x() * sr.deflectedDirection.x()
                             + sr.deflectedDirection.y() * sr.deflectedDirection.y());
    EXPECT_NEAR(length, 1.0, 1e-6);
}

TEST(SnellLawTest, TotalInternalReflection)
{
    QPointF incident(0.9, -0.436);
    QPointF normal(0.0, -1.0);

    SnellResult sr = deflectRay(incident, normal, 1.5, 1.0);
    EXPECT_TRUE(sr.totalInternalReflection);
}

TEST(SnellLawTest, SetDeflectionNormal)
{
    QPointF surfacePoint(0.0, 0.0);
    QPointF vecIn(0.5, -0.866);
    QPointF vecOut(0.866, -0.5);
    bool tir = false;

    QPointF norm = setDeflectionNormal(surfacePoint, vecIn, vecOut, 1.0, 1.5, tir);
    EXPECT_FALSE(tir);
}
