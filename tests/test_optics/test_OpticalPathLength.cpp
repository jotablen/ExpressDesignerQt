#include <gtest/gtest.h>
#include <optics/OpticalPathLength.h>

using namespace ExpressDesigner;

TEST(OpticalPathLengthTest, ComputeDirect)
{
    QPointF p1(0.0, 0.0);
    QPointF p2(3.0, 4.0);
    double index = 1.0;

    double opl = OpticalPathLength::compute(p1, p2, index);
    EXPECT_NEAR(opl, 5.0, 1e-9);
}

TEST(OpticalPathLengthTest, ComputeWithIndex)
{
    QPointF p1(0.0, 0.0);
    QPointF p2(3.0, 4.0);
    double index = 1.5;

    double opl = OpticalPathLength::compute(p1, p2, index);
    EXPECT_NEAR(opl, 7.5, 1e-9);
}

TEST(OpticalPathLengthTest, ComputeThroughSurface)
{
    QPointF source(0.0, 10.0);
    QPointF surfacePoint(0.0, 0.0);
    QPointF target(0.0, -10.0);
    double n1 = 1.0;
    double n2 = 1.5;

    double opl = OpticalPathLength::computeThroughSurface(source, surfacePoint, target, n1, n2);
    EXPECT_NEAR(opl, 10.0 + 15.0, 1e-9);
}

TEST(OpticalPathLengthTest, ComputePath)
{
    QVector<QPointF> path;
    path.append(QPointF(0.0, 0.0));
    path.append(QPointF(3.0, 4.0));
    path.append(QPointF(6.0, 4.0));
    QVector<double> indices;
    indices.append(1.0);
    indices.append(1.5);

    double opl = OpticalPathLength::computePath(path, indices);
    EXPECT_NEAR(opl, 5.0 + 4.5, 1e-9);
}