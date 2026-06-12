#include <gtest/gtest.h>
#include <core/ObjectFactory.h>
#include <core/PointObject.h>
#include <core/LineObject.h>
#include <core/ArcObject.h>
#include <core/CurveObject.h>

using namespace ExpressDesigner;

TEST(ObjectFactoryTest, CreatePoint)
{
    auto* obj = ObjectFactory::createFromType(ObjectType::Point, QStringLiteral("P1"), false);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->name(), QStringLiteral("P1"));
    EXPECT_EQ(obj->objectType(), ObjectType::Point);
    EXPECT_FALSE(obj->isWavefront());
    delete obj;
}

TEST(ObjectFactoryTest, CreateLine)
{
    auto* obj = ObjectFactory::createFromType(ObjectType::Line, QStringLiteral("L1"), false);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->name(), QStringLiteral("L1"));
    EXPECT_EQ(toBaseType(obj->objectType()), 0x002);
    delete obj;
}

TEST(ObjectFactoryTest, CreateArc)
{
    auto* obj = ObjectFactory::createFromType(ObjectType::Arc, QStringLiteral("A1"), false);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->name(), QStringLiteral("A1"));
    EXPECT_EQ(toBaseType(obj->objectType()), 0x003);
    delete obj;
}

TEST(ObjectFactoryTest, CreateCurve)
{
    auto* obj = ObjectFactory::createFromType(ObjectType::Curve, QStringLiteral("C1"), false);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->name(), QStringLiteral("C1"));
    EXPECT_EQ(toBaseType(obj->objectType()), 0x004);
    delete obj;
}

TEST(ObjectFactoryTest, CreateWavefront)
{
    auto* obj = ObjectFactory::createFromType(withWavefront(ObjectType::Curve), QStringLiteral("WF1"), true);
    ASSERT_NE(obj, nullptr);
    EXPECT_TRUE(obj->isWavefront());
    delete obj;
}

TEST(ObjectFactoryTest, DefaultName)
{
    auto* obj1 = ObjectFactory::createFromType(ObjectType::Point, QStringLiteral("P1"), false);
    auto* obj2 = ObjectFactory::createFromType(ObjectType::Point, QStringLiteral("P2"), false);
    ASSERT_NE(obj1, nullptr);
    ASSERT_NE(obj2, nullptr);
    EXPECT_NE(obj1->name(), obj2->name());
    delete obj1;
    delete obj2;
}

TEST(ObjectFactoryTest, CreateOperation)
{
    auto* op = ObjectFactory::createOperation(OperationType::CartesianOval, QStringLiteral("Op1"));
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->operationType(), OperationType::CartesianOval);
    delete op;
}