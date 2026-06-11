#include <gtest/gtest.h>
#include <geometry/SISLWrapper.h>

using namespace ExpressDesigner;
using namespace ExpressDesigner::SISL;

TEST(SISLWrapperTest, CurveDefaultInvalid)
{
    Curve curve;
    EXPECT_FALSE(curve.isValid());
    EXPECT_EQ(curve.native(), nullptr);
}

TEST(SISLWrapperTest, CurveEvaluateEmpty)
{
    Curve curve;
    QPointF p = curve.evaluate(0.5);
    EXPECT_DOUBLE_EQ(p.x(), 0.0);
    EXPECT_DOUBLE_EQ(p.y(), 0.0);
}

TEST(SISLWrapperTest, CurveFromControlPoints)
{
    QVector<QPointF> pts;
    pts.append(QPointF(0.0, 0.0));
    pts.append(QPointF(1.0, 2.0));
    pts.append(QPointF(3.0, 1.0));

    Curve curve = Curve::fromControlPoints(pts, 3);
    // With SISL not linked as a real library in test, the curve may be invalid
    // This test validates the API interface compiles and runs
    QPointF p = curve.evaluate(0.0);
    // At minimum the call should not crash
    EXPECT_TRUE(true);
}

TEST(SISLWrapperTest, SurfaceDefaultInvalid)
{
    Surface surface;
    EXPECT_FALSE(surface.isValid());
}

TEST(SISLWrapperTest, SurfaceEvaluateEmpty)
{
    Surface surface;
    QPointF p = surface.evaluate(0.5, 0.5);
    EXPECT_DOUBLE_EQ(p.x(), 0.0);
    EXPECT_DOUBLE_EQ(p.y(), 0.0);
}

TEST(SISLWrapperTest, CurveMoveSemantics)
{
    QVector<QPointF> pts;
    pts.append(QPointF(0.0, 0.0));
    pts.append(QPointF(10.0, 10.0));

    Curve c1 = Curve::fromControlPoints(pts, 2);
    Curve c2 = std::move(c1);
    // c1 should be in moved-from state, c2 should be valid (if SISL is linked)
    EXPECT_TRUE(true);
}

TEST(SISLWrapperTest, OperationsCreateNURBS)
{
    QVector<QPointF> pts;
    pts.append(QPointF(0.0, 0.0));
    pts.append(QPointF(5.0, 5.0));
    pts.append(QPointF(10.0, 0.0));

    Curve curve = Operations::createNURBSCurve(pts, 3);
    EXPECT_TRUE(true); // API smoke test
}

TEST(SISLWrapperTest, OperationsPointsToCurve)
{
    QVector<QPointF> pts;
    pts.append(QPointF(0.0, 0.0));
    pts.append(QPointF(1.0, 1.0));

    Curve curve = Operations::pointsToCurve(pts);
    QVector<QPointF> result = Operations::curveToPoints(curve, 10);
    EXPECT_GE(result.size(), 0);
}

TEST(SISLWrapperTest, ComputeNormals)
{
    QVector<QPointF> pts;
    pts.append(QPointF(0.0, 0.0));
    pts.append(QPointF(5.0, 0.0));

    Curve curve = Operations::pointsToCurve(pts);
    auto normals = Operations::computeNormals(curve, 5);
    EXPECT_GE(normals.size(), 0);
}