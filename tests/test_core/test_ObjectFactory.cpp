#include <gtest/gtest.h>
#include <core/ObjectFactory.h>
#include <core/PointObject.h>
#include <core/LineObject.h>
#include <core/ArcObject.h>
#include <core/CurveObject.h>
#include <core/ObjectTypes.h>

using namespace ExpressDesigner;

TEST(ObjectFactoryTest, CreatePoint)
{
    auto* obj = ObjectFactory::create(ObjectType::Point, QStringLiteral("P1"));
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->objectType(), ObjectType::Point);
    EXPECT_EQ(obj->name(), QStringLiteral("P1"));
    delete obj;
}

TEST(ObjectFactoryTest, CreateLine)
{
    auto* obj = ObjectFactory::create(ObjectType::Line, QStringLiteral("L1"));
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->objectType(), ObjectType::Line);
    EXPECT_EQ(obj->name(), QStringLiteral("L1"));
    delete obj;
}

TEST(ObjectFactoryTest, CreateArc)
{
    auto* obj = ObjectFactory::create(ObjectType::Arc, QStringLiteral("A1"));
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->objectType(), ObjectType::Arc);
    EXPECT_EQ(obj->name(), QStringLiteral("A1"));
    delete obj;
}

TEST(ObjectFactoryTest, CreateCurve)
{
    auto* obj = ObjectFactory::create(ObjectType::Curve, QStringLiteral("C1"));
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->objectType(), ObjectType::Curve);
    EXPECT_EQ(obj->name(), QStringLiteral("C1"));
    delete obj;
}

TEST(ObjectFactoryTest, CreateWavefront)
{
    auto* obj = ObjectFactory::create(withWavefront(ObjectType::Curve), QStringLiteral("WF1"));
    ASSERT_NE(obj, nullptr);
    EXPECT_TRUE(obj->isWavefront());
    delete obj;
}

TEST(ObjectFactoryTest, DefaultName)
{
    auto* obj1 = ObjectFactory::create(ObjectType::Point);
    auto* obj2 = ObjectFactory::create(ObjectType::Point);
    EXPECT_FALSE(obj1->name().isEmpty());
    EXPECT_FALSE(obj2->name().isEmpty());
    delete obj1;
    delete obj2;
}