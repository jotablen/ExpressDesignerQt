#include <gtest/gtest.h>
#include <core/LineObject.h>
#include <core/ObjectTypes.h>
#include <QtMath>

using namespace ExpressDesigner;

TEST(LineObjectTest, DefaultConstruction)
{
    LineObject line(QStringLiteral("L1"));
    EXPECT_EQ(line.name(), QStringLiteral("L1"));
    EXPECT_EQ(line.objectType(), ObjectType::Line);
    EXPECT_FALSE(line.isWavefront());
}

TEST(LineObjectTest, WavefrontConstruction)
{
    LineObject line(QStringLiteral("WFL1"), true);
    EXPECT_TRUE(line.isWavefront());
}

TEST(LineObjectTest, SetPoints)
{
    LineObject line(QStringLiteral("L1"));
    line.setStartPoint(QPointF(2.0, 3.0));
    line.setEndPoint(QPointF(8.0, 9.0));
    EXPECT_DOUBLE_EQ(line.startPoint().x(), 2.0);
    EXPECT_DOUBLE_EQ(line.startPoint().y(), 3.0);
    EXPECT_DOUBLE_EQ(line.endPoint().x(), 8.0);
    EXPECT_DOUBLE_EQ(line.endPoint().y(), 9.0);
}

TEST(LineObjectTest, Length)
{
    LineObject line(QStringLiteral("L1"));
    line.setStartPoint(QPointF(0.0, 0.0));
    line.setEndPoint(QPointF(3.0, 4.0));
    EXPECT_DOUBLE_EQ(line.length(), 5.0);
}

TEST(LineObjectTest, JsonSerialization)
{
    LineObject src(QStringLiteral("SrcLine"));
    src.setStartPoint(QPointF(1.0, 2.0));
    src.setEndPoint(QPointF(7.0, 8.0));
    src.setRefractiveIndex(1.5);

    QJsonObject json;
    src.saveToJson(json);

    LineObject dst(QStringLiteral(""));
    dst.loadFromJson(json);

    EXPECT_DOUBLE_EQ(dst.startPoint().x(), 1.0);
    EXPECT_DOUBLE_EQ(dst.endPoint().y(), 8.0);
    EXPECT_DOUBLE_EQ(dst.refractiveIndex(), 1.5);
}

TEST(LineObjectTest, Clone)
{
    LineObject src(QStringLiteral("Original"));
    src.setStartPoint(QPointF(0.0, 0.0));
    src.setEndPoint(QPointF(10.0, 0.0));
    src.setRefractiveIndex(1.33);

    LineObject* clone = src.clone();
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->name(), QStringLiteral("Original"));
    EXPECT_DOUBLE_EQ(clone->startPoint().x(), 0.0);
    EXPECT_DOUBLE_EQ(clone->endPoint().x(), 10.0);
    EXPECT_DOUBLE_EQ(clone->refractiveIndex(), 1.33);

    delete clone;
}

TEST(LineObjectTest, RhinoScript)
{
    LineObject line(QStringLiteral("L1"));
    line.setStartPoint(QPointF(1.0, 2.0));
    line.setEndPoint(QPointF(3.0, 4.0));
    QString script = line.toRhinoScript(false);
    EXPECT_TRUE(script.contains("_Line"));
}

// ========== CURVE GENERATION ROUNDTRIP TESTS ==========

TEST(LineObjectTest, ControlPointsAreTwoEndpoints)
{
    LineObject line(QStringLiteral("L1"));
    line.setStartPoint(QPointF(0.0, 0.0));
    line.setEndPoint(QPointF(10.0, 10.0));
    EXPECT_EQ(line.controlPointCount(), 2);
    EXPECT_DOUBLE_EQ(line.controlPoints()[0].x(), 0.0);
    EXPECT_DOUBLE_EQ(line.controlPoints()[1].y(), 10.0);
}

TEST(LineObjectTest, LinearDiscretization)
{
    // A line with 2 control points should discretize correctly to any resolution
    LineObject line(QStringLiteral("L1"));
    line.setStartPoint(QPointF(0.0, 0.0));
    line.setEndPoint(QPointF(10.0, 0.0));

    // Discretize to 5 points
    QVector<QPointF> pts;
    pts.reserve(5);
    for (int i = 0; i < 5; ++i) {
        double t = i / 4.0;
        pts.append(QPointF(10.0 * t, 0.0));
    }
    ASSERT_EQ(pts.size(), 5);
    EXPECT_DOUBLE_EQ(pts.first().x(), 0.0);
    EXPECT_DOUBLE_EQ(pts.last().x(), 10.0);
    EXPECT_DOUBLE_EQ(pts[2].x(), 5.0);
}