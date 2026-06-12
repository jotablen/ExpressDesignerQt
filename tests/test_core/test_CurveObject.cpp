#include <gtest/gtest.h>
#include <core/CurveObject.h>
#include <core/ObjectTypes.h>

using namespace ExpressDesigner;

TEST(CurveObjectTest, DefaultConstruction)
{
    CurveObject curve(QStringLiteral("C1"));
    EXPECT_EQ(curve.name(), QStringLiteral("C1"));
    EXPECT_EQ(curve.objectType(), ObjectType::Curve);
    EXPECT_FALSE(curve.isWavefront());
    EXPECT_EQ(curve.splineOrder(), 3);
    EXPECT_TRUE(curve.isOpen());
}

TEST(CurveObjectTest, WavefrontConstruction)
{
    CurveObject curve(QStringLiteral("WFC1"), true);
    EXPECT_TRUE(curve.isWavefront());
}

TEST(CurveObjectTest, ControlPoints)
{
    CurveObject curve(QStringLiteral("C1"));
    curve.addControlPoint(QPointF(0.0, 0.0));
    curve.addControlPoint(QPointF(5.0, 3.0));
    curve.addControlPoint(QPointF(10.0, 0.0));

    EXPECT_EQ(curve.controlPointCount(), 3);
    EXPECT_DOUBLE_EQ(curve.controlPoints()[1].x(), 5.0);
    EXPECT_DOUBLE_EQ(curve.controlPoints()[1].y(), 3.0);
}

TEST(CurveObjectTest, RemoveControlPoint)
{
    CurveObject curve(QStringLiteral("C1"));
    curve.addControlPoint(QPointF(0.0, 0.0));
    curve.addControlPoint(QPointF(5.0, 3.0));
    curve.addControlPoint(QPointF(10.0, 0.0));
    EXPECT_EQ(curve.controlPointCount(), 3);

    curve.removeControlPoint(1);
    EXPECT_EQ(curve.controlPointCount(), 2);
    EXPECT_DOUBLE_EQ(curve.controlPoints()[1].x(), 10.0);
}

TEST(CurveObjectTest, SplineOrder)
{
    CurveObject curve(QStringLiteral("C1"));
    curve.setSplineOrder(5);
    EXPECT_EQ(curve.splineOrder(), 5);
}

TEST(CurveObjectTest, OpenClosed)
{
    CurveObject curve(QStringLiteral("C1"));
    EXPECT_TRUE(curve.isOpen());
    curve.setOpen(false);
    EXPECT_FALSE(curve.isOpen());
}

TEST(CurveObjectTest, JsonSerialization)
{
    CurveObject src(QStringLiteral("SrcCurve"));
    src.addControlPoint(QPointF(0.0, 0.0));
    src.addControlPoint(QPointF(5.0, 5.0));
    src.addControlPoint(QPointF(10.0, 0.0));
    src.setSplineOrder(4);
    src.setOpen(false);
    src.setRefractiveIndex(1.5);

    QJsonObject json;
    src.saveToJson(json);

    CurveObject dst(QStringLiteral(""));
    dst.loadFromJson(json);

    EXPECT_EQ(dst.name(), QStringLiteral("SrcCurve"));
    EXPECT_EQ(dst.controlPointCount(), 3);
    EXPECT_EQ(dst.splineOrder(), 4);
    EXPECT_FALSE(dst.isOpen());
    EXPECT_DOUBLE_EQ(dst.refractiveIndex(), 1.5);
}

TEST(CurveObjectTest, Clone)
{
    CurveObject src(QStringLiteral("Original"));
    src.addControlPoint(QPointF(1.0, 2.0));
    src.addControlPoint(QPointF(3.0, 4.0));
    src.setSplineOrder(4);
    src.setRefractiveIndex(1.33);

    CurveObject* clone = src.clone();
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->name(), QStringLiteral("Original"));
    EXPECT_EQ(clone->controlPointCount(), 2);
    EXPECT_EQ(clone->splineOrder(), 4);
    EXPECT_DOUBLE_EQ(clone->refractiveIndex(), 1.33);

    delete clone;
}

TEST(CurveObjectTest, RhinoScript)
{
    CurveObject curve(QStringLiteral("C1"));
    curve.addControlPoint(QPointF(0.0, 0.0));
    curve.addControlPoint(QPointF(5.0, 5.0));
    QString script = curve.toRhinoScript(false);
    EXPECT_TRUE(script.contains("_InterpCrv"));
}

// ========== CURVE GENERATION ROUNDTRIP TESTS ==========

TEST(CurveObjectTest, DiscretizeFromControlPoints)
{
    CurveObject curve(QStringLiteral("C1"));
    curve.addControlPoint(QPointF(0.0, 0.0));
    curve.addControlPoint(QPointF(10.0, 10.0));
    curve.addControlPoint(QPointF(20.0, 0.0));

    // Discretize to 101 points along the polyline
    QVector<QPointF> pts = curve.discretize(101);
    ASSERT_EQ(pts.size(), 101);
    // First point should be first control point
    EXPECT_DOUBLE_EQ(pts.first().x(), 0.0);
    EXPECT_DOUBLE_EQ(pts.first().y(), 0.0);
    // Last point should be last control point
    EXPECT_DOUBLE_EQ(pts.last().x(), 20.0);
    EXPECT_DOUBLE_EQ(pts.last().y(), 0.0);
    // Middle point should be (10, 10)
    EXPECT_NEAR(pts[50].x(), 10.0, 0.01);
    EXPECT_NEAR(pts[50].y(), 10.0, 0.01);
}

TEST(CurveObjectTest, DiscretizeRoundtrip)
{
    // Save curve, reload, discretize — should get same points
    CurveObject src(QStringLiteral("Src"));
    src.addControlPoint(QPointF(0.0, 0.0));
    src.addControlPoint(QPointF(5.0, 8.0));
    src.addControlPoint(QPointF(10.0, 0.0));
    src.setSplineOrder(3);

    QJsonObject json;
    src.saveToJson(json);

    CurveObject dst(QStringLiteral(""));
    dst.loadFromJson(json);

    EXPECT_EQ(dst.controlPointCount(), 3);
    EXPECT_DOUBLE_EQ(dst.controlPoints()[1].x(), 5.0);
    EXPECT_DOUBLE_EQ(dst.controlPoints()[1].y(), 8.0);

    auto srcPts = src.discretize(51);
    auto dstPts = dst.discretize(51);
    ASSERT_EQ(srcPts.size(), dstPts.size());
    for (int i = 0; i < srcPts.size(); ++i) {
        EXPECT_DOUBLE_EQ(srcPts[i].x(), dstPts[i].x());
        EXPECT_DOUBLE_EQ(srcPts[i].y(), dstPts[i].y());
    }
}

TEST(CurveObjectTest, DiscretizeEmpty)
{
    CurveObject curve(QStringLiteral("Empty"));
    auto pts = curve.discretize(10);
    EXPECT_TRUE(pts.isEmpty());
}

TEST(CurveObjectTest, DiscretizeSinglePoint)
{
    CurveObject curve(QStringLiteral("Single"));
    curve.addControlPoint(QPointF(5.0, 3.0));
    auto pts = curve.discretize(5);
    ASSERT_EQ(pts.size(), 5);
    for (const auto& pt : pts) {
        EXPECT_DOUBLE_EQ(pt.x(), 5.0);
        EXPECT_DOUBLE_EQ(pt.y(), 3.0);
    }
}