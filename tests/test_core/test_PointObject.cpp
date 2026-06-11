#include <gtest/gtest.h>
#include <core/PointObject.h>
#include <core/ObjectTypes.h>

using namespace ExpressDesigner;

TEST(PointObjectTest, DefaultConstruction)
{
    PointObject pt(QStringLiteral("P1"));
    EXPECT_EQ(pt.name(), QStringLiteral("P1"));
    EXPECT_EQ(pt.objectType(), ObjectType::Point);
    EXPECT_FALSE(pt.isWavefront());
    EXPECT_TRUE(pt.isVisible());
}

TEST(PointObjectTest, WavefrontConstruction)
{
    PointObject pt(QStringLiteral("WFP1"), true);
    EXPECT_TRUE(pt.isWavefront());
}

TEST(PointObjectTest, Position)
{
    PointObject pt(QStringLiteral("P1"));
    pt.setPosition(QPointF(3.0, 4.0));
    EXPECT_DOUBLE_EQ(pt.position().x(), 3.0);
    EXPECT_DOUBLE_EQ(pt.position().y(), 4.0);
}

TEST(PointObjectTest, Radius)
{
    PointObject pt(QStringLiteral("P1"));
    pt.setRadius(1.5);
    EXPECT_DOUBLE_EQ(pt.radius(), 1.5);
}

TEST(PointObjectTest, JsonSerialization)
{
    PointObject src(QStringLiteral("SrcPt"));
    src.setPosition(QPointF(10.0, 20.0));
    src.setRadius(0.75);
    src.setRefractiveIndex(1.5);

    QJsonObject json;
    src.saveToJson(json);

    PointObject dst(QStringLiteral(""));
    dst.loadFromJson(json);

    EXPECT_DOUBLE_EQ(dst.position().x(), 10.0);
    EXPECT_DOUBLE_EQ(dst.position().y(), 20.0);
    EXPECT_DOUBLE_EQ(dst.radius(), 0.75);
    EXPECT_DOUBLE_EQ(dst.refractiveIndex(), 1.5);
}

TEST(PointObjectTest, Clone)
{
    PointObject src(QStringLiteral("Original"));
    src.setPosition(QPointF(5.0, 5.0));
    src.setRadius(2.0);
    src.setRefractiveIndex(1.33);

    PointObject* clone = src.clone();
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->name(), QStringLiteral("Original"));
    EXPECT_DOUBLE_EQ(clone->position().x(), 5.0);
    EXPECT_DOUBLE_EQ(clone->position().y(), 5.0);
    EXPECT_DOUBLE_EQ(clone->radius(), 2.0);
    EXPECT_DOUBLE_EQ(clone->refractiveIndex(), 1.33);

    delete clone;
}

TEST(PointObjectTest, RhinoScript)
{
    PointObject pt(QStringLiteral("P1"));
    pt.setPosition(QPointF(2.5, 3.5));
    QString script = pt.toRhinoScript(false);
    EXPECT_TRUE(script.contains("_Point"));
    EXPECT_TRUE(script.contains("2.5"));
    EXPECT_TRUE(script.contains("3.5"));
}