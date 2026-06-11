#include <gtest/gtest.h>
#include <core/CustomObject.h>

using namespace ExpressDesigner;

TEST(CustomObjectTest, DefaultConstruction)
{
    CustomObject obj(ObjectType::Curve, QStringLiteral("TestCurve"));
    EXPECT_EQ(obj.name(), QStringLiteral("TestCurve"));
    EXPECT_DOUBLE_EQ(obj.refractiveIndex(), 1.0);
    EXPECT_EQ(obj.controlPointCount(), 0);
}

TEST(CustomObjectTest, ControlPoints)
{
    CustomObject obj(ObjectType::Curve, QStringLiteral("C1"));
    obj.addControlPoint(QPointF(1.0, 2.0));
    obj.addControlPoint(QPointF(3.0, 4.0));
    EXPECT_EQ(obj.controlPointCount(), 2);
    EXPECT_DOUBLE_EQ(obj.controlPoints()[0].x(), 1.0);
    EXPECT_DOUBLE_EQ(obj.controlPoints()[1].y(), 4.0);
}

TEST(CustomObjectTest, RemoveControlPoint)
{
    CustomObject obj(ObjectType::Curve, QStringLiteral("C1"));
    obj.addControlPoint(QPointF(0, 0));
    obj.addControlPoint(QPointF(1, 1));
    obj.addControlPoint(QPointF(2, 2));
    obj.removeControlPoint(1);
    EXPECT_EQ(obj.controlPointCount(), 2);
    EXPECT_DOUBLE_EQ(obj.controlPoints()[1].x(), 2.0);
}

TEST(CustomObjectTest, ClearPoints)
{
    CustomObject obj(ObjectType::Line, QStringLiteral("L1"));
    obj.addControlPoint(QPointF(0, 0));
    obj.addControlPoint(QPointF(5, 5));
    obj.clearControlPoints();
    EXPECT_EQ(obj.controlPointCount(), 0);
}

TEST(CustomObjectTest, RefractiveIndex)
{
    CustomObject obj(ObjectType::Point, QStringLiteral("P1"));
    obj.setRefractiveIndex(1.5);
    EXPECT_DOUBLE_EQ(obj.refractiveIndex(), 1.5);
}

TEST(CustomObjectTest, ReferencePoint)
{
    CustomObject obj(ObjectType::Curve, QStringLiteral("C1"));
    obj.setReferencePoint(QPointF(10.0, 20.0));
    EXPECT_DOUBLE_EQ(obj.referencePoint().x(), 10.0);
    EXPECT_DOUBLE_EQ(obj.referencePoint().y(), 20.0);
}

TEST(CustomObjectTest, NormalsComputation)
{
    CustomObject obj(ObjectType::Line, QStringLiteral("L1"));
    obj.addControlPoint(QPointF(0, 0));
    obj.addControlPoint(QPointF(10, 0));
    obj.setNormalParams(5, 2.0);
    auto normals = obj.computeNormals();
    EXPECT_EQ(normals.size(), 5);
}

TEST(CustomObjectTest, Serialization)
{
    CustomObject src(ObjectType::Curve, QStringLiteral("SrcCurve"));
    src.setRefractiveIndex(1.7);
    src.addControlPoint(QPointF(1, 2));
    src.addControlPoint(QPointF(3, 4));
    src.setReferencePoint(QPointF(0, 0));

    QJsonObject json;
    src.saveToJson(json);

    CustomObject dst(ObjectType::Curve, QStringLiteral(""));
    dst.loadFromJson(json);

    EXPECT_EQ(dst.name(), QStringLiteral("SrcCurve"));
    EXPECT_DOUBLE_EQ(dst.refractiveIndex(), 1.7);
    EXPECT_EQ(dst.controlPointCount(), 2);
}