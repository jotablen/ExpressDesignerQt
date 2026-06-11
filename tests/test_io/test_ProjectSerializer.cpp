#include <gtest/gtest.h>
#include <io/ProjectSerializer.h>
#include <core/Project.h>
#include <core/PointObject.h>
#include <core/CurveObject.h>

#include <QTemporaryFile>
#include <QDir>

using namespace ExpressDesigner;

TEST(ProjectSerializerTest, SaveAndLoad)
{
    Project src;
    src.setName(QStringLiteral("SerializerTest"));
    auto* pt = new PointObject(QStringLiteral("P1"));
    pt->setPosition(QPointF(3.0, 4.0));
    pt->setRefractiveIndex(1.5);
    src.addDataObject(pt);

    auto* curve = new CurveObject(QStringLiteral("C1"));
    curve->addControlPoint(QPointF(0.0, 0.0));
    curve->addControlPoint(QPointF(5.0, 5.0));
    curve->addControlPoint(QPointF(10.0, 0.0));
    src.addDataObject(curve);

    // Save to temp file
    QString tempPath = QDir::temp().absoluteFilePath(QStringLiteral("test_project.json"));
    ASSERT_TRUE(ProjectSerializer::save(src, tempPath));

    // Load back
    Project* loaded = ProjectSerializer::load(tempPath);
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->name(), QStringLiteral("SerializerTest"));
    EXPECT_EQ(loaded->dataObjectCount(), 2);
    EXPECT_EQ(loaded->dataObjectAt(0)->name(), QStringLiteral("P1"));
    EXPECT_EQ(loaded->dataObjectAt(1)->name(), QStringLiteral("C1"));

    delete loaded;
    QFile::remove(tempPath);
}

TEST(ProjectSerializerTest, SaveEmptyProject)
{
    Project proj;
    proj.setName(QStringLiteral("Empty"));

    QString tempPath = QDir::temp().absoluteFilePath(QStringLiteral("test_empty.json"));
    ASSERT_TRUE(ProjectSerializer::save(proj, tempPath));

    Project* loaded = ProjectSerializer::load(tempPath);
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->name(), QStringLiteral("Empty"));
    EXPECT_EQ(loaded->dataObjectCount(), 0);

    delete loaded;
    QFile::remove(tempPath);
}

TEST(ProjectSerializerTest, LoadNonExistentFile)
{
    Project* loaded = ProjectSerializer::load(QStringLiteral("non_existent_file.json"));
    EXPECT_EQ(loaded, nullptr);
}

TEST(ProjectSerializerTest, SaveHistory)
{
    HistoryManager history;
    history.addEntry(QStringLiteral("project_created"), QStringLiteral("TestProject"));
    history.addEntry(QStringLiteral("object_created"), QStringLiteral("Point1 (Point)"));
    EXPECT_EQ(history.entries().size(), 2);

    QString tempPath = QDir::temp().absoluteFilePath(QStringLiteral("test_history.json"));
    ASSERT_TRUE(ProjectSerializer::saveHistory(history, tempPath));

    HistoryManager loaded;
    ProjectSerializer::loadHistory(loaded, tempPath);
    EXPECT_EQ(loaded.entries().size(), 2);

    QFile::remove(tempPath);
}