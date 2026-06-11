#include <gtest/gtest.h>
#include <core/Project.h>
#include <core/PointObject.h>
#include <core/LineObject.h>
#include <core/CurveObject.h>
#include <core/ArcObject.h>

using namespace ExpressDesigner;

TEST(ProjectTest, DefaultConstruction)
{
    Project proj;
    EXPECT_FALSE(proj.name().isEmpty());
    EXPECT_FALSE(proj.uuid().isNull());
    EXPECT_EQ(proj.dataObjectCount(), 0);
    EXPECT_EQ(proj.resultObjectCount(), 0);
}

TEST(ProjectTest, SetName)
{
    Project proj;
    proj.setName(QStringLiteral("TestProject"));
    EXPECT_EQ(proj.name(), QStringLiteral("TestProject"));
}

TEST(ProjectTest, AddDataObject)
{
    Project proj;
    auto* pt = new PointObject(QStringLiteral("P1"));
    proj.addDataObject(pt);

    EXPECT_EQ(proj.dataObjectCount(), 1);
    EXPECT_EQ(proj.dataObjects()[0]->name(), QStringLiteral("P1"));
}

TEST(ProjectTest, AddResultObject)
{
    Project proj;
    auto* curve = new CurveObject(QStringLiteral("ResultCurve"));
    curve->setObjectType(withResult(ObjectType::Curve));
    proj.addResultObject(curve);

    EXPECT_EQ(proj.resultObjectCount(), 1);
    EXPECT_EQ(proj.resultObjects()[0]->name(), QStringLiteral("ResultCurve"));
}

TEST(ProjectTest, RemoveDataObject)
{
    Project proj;
    auto* pt = new PointObject(QStringLiteral("P1"));
    proj.addDataObject(pt);
    EXPECT_EQ(proj.dataObjectCount(), 1);

    proj.removeDataObject(pt);
    EXPECT_EQ(proj.dataObjectCount(), 0);
}

TEST(ProjectTest, FindByUuid)
{
    Project proj;
    auto* line = new LineObject(QStringLiteral("L1"));
    line->setStartPoint(QPointF(0, 0));
    line->setEndPoint(QPointF(10, 0));
    QUuid id = line->uuid();
    proj.addDataObject(line);

    auto* found = proj.findObject(id);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name(), QStringLiteral("L1"));
}

TEST(ProjectTest, ClearAll)
{
    Project proj;
    proj.addDataObject(new PointObject(QStringLiteral("P1")));
    proj.addDataObject(new PointObject(QStringLiteral("P2")));
    proj.addResultObject(new CurveObject(QStringLiteral("R1")));
    EXPECT_EQ(proj.dataObjectCount(), 2);
    EXPECT_EQ(proj.resultObjectCount(), 1);

    proj.clearAll();
    EXPECT_EQ(proj.dataObjectCount(), 0);
    EXPECT_EQ(proj.resultObjectCount(), 0);
}

TEST(ProjectTest, JsonSerialization)
{
    Project src;
    src.setName(QStringLiteral("SaveTest"));
    auto* pt = new PointObject(QStringLiteral("P1"));
    pt->setPosition(QPointF(3.0, 4.0));
    src.addDataObject(pt);

    QJsonObject json;
    src.saveToJson(json);

    Project dst;
    dst.loadFromJson(json);

    EXPECT_EQ(dst.name(), QStringLiteral("SaveTest"));
    EXPECT_EQ(dst.dataObjectCount(), 1);
    EXPECT_EQ(dst.dataObjects()[0]->name(), QStringLiteral("P1"));
}
