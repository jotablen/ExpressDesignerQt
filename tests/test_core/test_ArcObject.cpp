#include <gtest/gtest.h>
#include <core/ArcObject.h>
#include <core/ObjectTypes.h>

using namespace ExpressDesigner;

TEST(ArcObjectTest, DefaultConstruction)
{
    ArcObject arc(QStringLiteral("A1"));
    EXPECT_EQ(arc.name(), QStringLiteral("A1"));
    EXPECT_EQ(arc.objectType(), ObjectType::Arc);
    EXPECT_FALSE(arc.isWavefront());
    EXPECT_DOUBLE_EQ(arc.radius(), 1.0);
    EXPECT_DOUBLE_EQ(arc.startAngle(), 0.0);
    EXPECT_DOUBLE_EQ(arc.endAngle(), 90.0);
    EXPECT_EQ(arc.numPoints(), 50);
}

TEST(ArcObjectTest, WavefrontConstruction)
{
    ArcObject arc(QStringLiteral("WFA1"), true);
    EXPECT_TRUE(arc.isWavefront());
}

TEST(ArcObjectTest, SetParameters)
{
    ArcObject arc(QStringLiteral("A1"));
    arc.setCenter(QPointF(2.0, 3.0));
    arc.setRadius(8.0);
    arc.setStartAngle(30.0);
    arc.setEndAngle(150.0);
    arc.setNumPoints(100);

    EXPECT_DOUBLE_EQ(arc.center().x(), 2.0);
    EXPECT_DOUBLE_EQ(arc.center().y(), 3.0);
    EXPECT_DOUBLE_EQ(arc.radius(), 8.0);
    EXPECT_DOUBLE_EQ(arc.startAngle(), 30.0);
    EXPECT_DOUBLE_EQ(arc.endAngle(), 150.0);
    EXPECT_EQ(arc.numPoints(), 100);
}

TEST(ArcObjectTest, ControlPointsGenerated)
{
    ArcObject arc(QStringLiteral("A1"));
    arc.setCenter(QPointF(0.0, 0.0));
    arc.setRadius(5.0);
    arc.setStartAngle(0.0);
    arc.setEndAngle(360.0);
    arc.setNumPoints(72);
    arc.setControlPoints(arc.generateArcPoints());
    EXPECT_EQ(arc.controlPointCount(), 72);
    // Check first and last points
    EXPECT_NEAR(arc.controlPoints().first().x(), 5.0, 0.01);
    EXPECT_NEAR(arc.controlPoints().last().x(), 5.0, 0.01);
}

TEST(ArcObjectTest, JsonSerialization)
{
    ArcObject src(QStringLiteral("SrcArc"));
    src.setCenter(QPointF(1.0, 2.0));
    src.setRadius(5.0);
    src.setStartAngle(0.0);
    src.setEndAngle(90.0);
    src.setNumPoints(50);
    src.setRefractiveIndex(1.5);

    QJsonObject json;
    src.saveToJson(json);

    ArcObject dst(QStringLiteral(""));
    dst.loadFromJson(json);

    EXPECT_DOUBLE_EQ(dst.center().x(), 1.0);
    EXPECT_DOUBLE_EQ(dst.center().y(), 2.0);
    EXPECT_DOUBLE_EQ(dst.radius(), 5.0);
    EXPECT_DOUBLE_EQ(dst.startAngle(), 0.0);
    EXPECT_DOUBLE_EQ(dst.endAngle(), 90.0);
    EXPECT_DOUBLE_EQ(dst.refractiveIndex(), 1.5);
}

TEST(ArcObjectTest, Clone)
{
    ArcObject src(QStringLiteral("Original"));
    src.setCenter(QPointF(0.0, 0.0));
    src.setRadius(10.0);
    src.setStartAngle(-45.0);
    src.setEndAngle(45.0);
    src.setNumPoints(30);
    src.setRefractiveIndex(1.33);

    ArcObject* clone = src.clone();
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->name(), QStringLiteral("Original"));
    EXPECT_DOUBLE_EQ(clone->radius(), 10.0);
    EXPECT_DOUBLE_EQ(clone->startAngle(), -45.0);
    EXPECT_DOUBLE_EQ(clone->refractiveIndex(), 1.33);

    delete clone;
}

TEST(ArcObjectTest, RhinoScript)
{
    ArcObject arc(QStringLiteral("A1"));
    arc.setCenter(QPointF(0.0, 0.0));
    arc.setRadius(5.0);
    arc.setStartAngle(0.0);
    arc.setEndAngle(180.0);
    QString script = arc.toRhinoScript(false);
    EXPECT_TRUE(script.contains("_Arc"));
}

// ========== CURVE GENERATION ROUNDTRIP TESTS ==========
// Verify that each object type generates correct 2D control points
// and that those points can regenerate the same curves.

TEST(ArcObjectTest, CurveGenerationRoundtrip)
{
    // Create an arc, generate points, verify geometry
    ArcObject arc(QStringLiteral("TestArc"));
    arc.setCenter(QPointF(1.0, 2.0));
    arc.setRadius(10.0);
    arc.setStartAngle(0.0);
    arc.setEndAngle(90.0);
    arc.setNumPoints(100);
    arc.setControlPoints(arc.generateArcPoints());

    ASSERT_EQ(arc.controlPointCount(), 100);
    // All points should be exactly radius away from center
    for (const auto& pt : arc.controlPoints()) {
        double dist = std::sqrt(std::pow(pt.x() - 1.0, 2) + std::pow(pt.y() - 2.0, 2));
        EXPECT_NEAR(dist, 10.0, 0.01);
    }
    // First point = (1+10, 2+0) = (11, 2)
    EXPECT_NEAR(arc.controlPoints().first().x(), 11.0, 0.01);
    EXPECT_NEAR(arc.controlPoints().first().y(), 2.0, 0.01);
    // Last point = (1+0, 2+10) = (1, 12)
    EXPECT_NEAR(arc.controlPoints().last().x(), 1.0, 0.01);
    EXPECT_NEAR(arc.controlPoints().last().y(), 12.0, 0.01);
}

TEST(ArcObjectTest, ArcPointsRegeneration)
{
    // Roundtrip: generate arc points, reload arc params, regenerate — should match
    ArcObject src(QStringLiteral("Src"));
    src.setCenter(QPointF(5.0, 0.0));
    src.setRadius(8.0);
    src.setStartAngle(30.0);
    src.setEndAngle(210.0);
    src.setNumPoints(60);
    src.setControlPoints(src.generateArcPoints());

    QJsonObject json;
    src.saveToJson(json);

    ArcObject dst(QStringLiteral(""));
    dst.loadFromJson(json);
    dst.setControlPoints(dst.generateArcPoints());

    ASSERT_EQ(dst.controlPointCount(), 60);
    EXPECT_DOUBLE_EQ(dst.center().x(), 5.0);
    EXPECT_DOUBLE_EQ(dst.radius(), 8.0);
    EXPECT_DOUBLE_EQ(dst.startAngle(), 30.0);
    EXPECT_DOUBLE_EQ(dst.endAngle(), 210.0);
}