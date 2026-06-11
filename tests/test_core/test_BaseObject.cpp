#include <gtest/gtest.h>
#include <core/BaseObject.h>
#include <core/ObjectTypes.h>

using namespace ExpressDesigner;

TEST(BaseObjectTest, DefaultConstruction)
{
    BaseObject obj(ObjectType::Point, QStringLiteral("TestPoint"));
    EXPECT_EQ(obj.name(), QStringLiteral("TestPoint"));
    EXPECT_EQ(obj.objectType(), ObjectType::Point);
    EXPECT_FALSE(obj.uuid().isNull());
    EXPECT_TRUE(obj.isVisible());
}

TEST(BaseObjectTest, WavefrontDetection)
{
    BaseObject wf(ObjectType::Curve, QStringLiteral("WF1"));
    wf.setObjectType(withWavefront(ObjectType::Curve));
    EXPECT_TRUE(wf.isWavefront());
    EXPECT_FALSE(wf.isResult());
}

TEST(BaseObjectTest, ResultDetection)
{
    BaseObject res(ObjectType::Curve, QStringLiteral("Result1"));
    res.setObjectType(withResult(ObjectType::Curve));
    EXPECT_TRUE(res.isResult());
}

TEST(BaseObjectTest, VisibilityToggle)
{
    BaseObject obj(ObjectType::Point, QStringLiteral("P1"));
    EXPECT_TRUE(obj.isVisible());
    obj.setVisible(false);
    EXPECT_FALSE(obj.isVisible());
    obj.setVisible(true);
    EXPECT_TRUE(obj.isVisible());
}

TEST(BaseObjectTest, NormalParameters)
{
    BaseObject obj(ObjectType::Line, QStringLiteral("L1"));
    EXPECT_FALSE(obj.showNormals());
    obj.setShowNormals(true);
    EXPECT_TRUE(obj.showNormals());
    obj.setNormalParams(20, 2.5);
    EXPECT_EQ(obj.normalRaysQty(), 20);
    EXPECT_DOUBLE_EQ(obj.normalRaysLen(), 2.5);
}

TEST(BaseObjectTest, NormalFlipped)
{
    BaseObject obj(ObjectType::Curve, QStringLiteral("C1"));
    EXPECT_FALSE(obj.isNormalFlipped());
    obj.setNormalFlipped(true);
    EXPECT_TRUE(obj.isNormalFlipped());
    obj.flipNormal();
    EXPECT_FALSE(obj.isNormalFlipped());
}

TEST(BaseObjectTest, JsonSerialization)
{
    BaseObject src(ObjectType::Arc, QStringLiteral("Arc1"));
    src.setVisible(false);
    src.setNormalParams(15, 3.0);
    src.setNormalFlipped(true);

    QJsonObject json;
    src.saveToJson(json);

    BaseObject dst(ObjectType::Curve, QStringLiteral(""));
    dst.loadFromJson(json);

    EXPECT_EQ(dst.name(), QStringLiteral("Arc1"));
    EXPECT_FALSE(dst.isVisible());
    EXPECT_EQ(dst.normalRaysQty(), 15);
    EXPECT_DOUBLE_EQ(dst.normalRaysLen(), 3.0);
    EXPECT_TRUE(dst.isNormalFlipped());
}