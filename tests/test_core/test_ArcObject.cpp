#include <gtest/gtest.h>
#include <core/ArcObject.h>
#include <core/ObjectTypes.h>

using namespace ExpressDesigner;

TEST(ArcObjectTest, DefaultConstruction)
{
    ArcObject arc(QStringLiteral("A1"), QPointF(0.0, 0.0), 5.0, 0.0, 180.0, 50);
    EXPECT_EQ(arc.name(), QStringLiteral("A1"));
    EXPECT_EQ(arc.objectType(), ObjectType::Arc);
    EXPECT_DOUBLE_EQ(arc.center().x(), 0.0);
    EXPECT_DOUBLE_EQ(arc.center().y(), 0.0);
    EXPECT_DOUBLE_EQ(arc.radius(), 5.0);
    EXPECT_DOUBLE_EQ(arc.startAngle(), 0.0);
    EXPECT_DOUBLE_EQ(arc.endAngle(), 180.0);
    EXPECT_EQ(arc.numPoints(), 50);
}

TEST(ArcObjectTest, WavefrontConstruction)
{
    ArcObject arc(QStringLiteral("WFA1"), QPointF(0.0, 0.0), 10.0, 0.0, 90.0, 30, true);
    EXPECT_TRUE(arc.isWavefront());
}

TEST(ArcObjectTest, SetParameters)
{
    ArcObject arc(QStringLiteral("A1"), QPointF(0.0, 0.0), 5.0, 0.0, 180.0);
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
    ArcObject arc(QStringLiteral("A1"), QPointF(0.0, 0.0), 5.0, 0.0, 360.0, 72);
    EXPECT_EQ(arc.controlPointCount(), 72);
}

TEST(ArcObjectTest, JsonSerialization)
{
    ArcObject src(QStringLiteral("SrcArc"), QPointF(1.0, 2.0), 5.0, 0.0, 90.0, 50);
    src.setRefractiveIndex(1.5);

    QJsonObject json;
    src.saveToJson(json);

    ArcObject dst(QStringLiteral(""), QPointF(0.0, 0.0), 1.0, 0.0, 0.0);
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
    ArcObject src(QStringLiteral("Original"), QPointF(0.0, 0.0), 10.0, -45.0, 45.0, 30);
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
    ArcObject arc(QStringLiteral("A1"), QPointF(0.0, 0.0), 5.0, 0.0, 180.0);
    QString script = arc.toRhinoScript(false);
    EXPECT_TRUE(script.contains("_Arc"));
}