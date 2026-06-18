#include <gtest/gtest.h>
#include <geometry/SISLWrapper.h>
#include <QtMath>

using namespace ExpressDesigner::Geometry;

TEST(SISLCurveTest, closestPoint_horizontalLine)
{
    QVector<QPointF> pts = { {0, 0}, {1, 0}, {2, 0}, {3, 0} };
    SISLCurve curve(pts, 3, true);

    double t;
    QPointF cp = curve.closestPoint({1.5, 0.5}, &t);
    EXPECT_NEAR(cp.x(), 1.5, 1e-9);
    EXPECT_NEAR(cp.y(), 0.0, 1e-9);

    cp = curve.closestPoint({0.5, -1.0}, &t);
    EXPECT_NEAR(cp.x(), 0.5, 1e-9);
    EXPECT_NEAR(cp.y(), 0.0, 1e-9);
}

TEST(SISLCurveTest, rayIntersection_horizontalLine)
{
    QVector<QPointF> pts = { {0, 0}, {1, 0}, {2, 0}, {3, 0} };
    SISLCurve curve(pts, 3, true);

    double d = curve.rayIntersection({1.0, 1.0}, {0.0, -1.0});
    EXPECT_NEAR(d, 1.0, 1e-9);

    d = curve.rayIntersection({1.0, -1.0}, {0.0, 1.0});
    EXPECT_NEAR(d, 1.0, 1e-9);

    d = curve.rayIntersection({0.0, 1.0}, {1.0, 0.0});
    EXPECT_LT(d, 0.0);  // no intersection

    d = curve.rayIntersection({1.0, 1.0}, {0.0, 1.0});
    EXPECT_LT(d, 0.0);
}

TEST(SISLCurveTest, rayIntersection_verticalLine)
{
    QVector<QPointF> pts = { {0, 0}, {0, 1}, {0, 2}, {0, 3} };
    SISLCurve curve(pts, 3, true);

    double d = curve.rayIntersection({-1.0, 1.0}, {1.0, 0.0});
    EXPECT_NEAR(d, 1.0, 1e-9);

    d = curve.rayIntersection({1.0, 1.0}, {-1.0, 0.0});
    EXPECT_NEAR(d, 1.0, 1e-9);

    d = curve.rayIntersection({1.0, 1.0}, {0.0, 1.0});
    EXPECT_LT(d, 0.0);
}

TEST(SISLCurveTest, rayIntersection_diagonalV)
{
    QVector<QPointF> pts = { {0, 0}, {1, -1}, {2, 0} };
    SISLCurve curve(pts, 3, true);

    double d = curve.rayIntersection({1.0, 1.0}, {0.0, -1.0});
    EXPECT_GT(d, 0.0);
    QPointF hit = {1.0, 1.0 - d};
    EXPECT_NEAR(hit.x(), 1.0, 0.1);
    EXPECT_NEAR(hit.y(), -1.0, 0.1);

    d = curve.rayIntersection({0.0, 0.0}, {1.0, -0.5});
    EXPECT_GT(d, 0.0);
}

TEST(SISLCurveTest, normalAt_consistency)
{
    QVector<QPointF> pts = { {0, 0}, {1, 0}, {2, 1}, {3, 1} };
    SISLCurve curve(pts, 3, true);

    QPointF n1 = curve.normal(0.5);
    QPointF n2 = curve.normalAt(curve.evaluate(0.5));
    EXPECT_NEAR(n1.x(), n2.x(), 1e-9);
    EXPECT_NEAR(n1.y(), n2.y(), 1e-9);
}

TEST(SISLCurveTest, closestPoint_vs_discrete)
{
    QVector<QPointF> pts = { {0, 0}, {1, 2}, {2, 1}, {3, 3} };
    SISLCurve curve(pts, 3, true);

    QPointF query(1.8, 0.5);
    double t;
    QPointF cp = curve.closestPoint(query, &t);

    auto dense = curve.evaluateAll(10000);
    int bestI = 0;
    double bestD2 = 1e18;
    for (int i = 0; i < dense.size(); ++i) {
        double d2 = (dense[i].x() - query.x()) * (dense[i].x() - query.x())
                  + (dense[i].y() - query.y()) * (dense[i].y() - query.y());
        if (d2 < bestD2) { bestD2 = d2; bestI = i; }
    }
    QPointF discCp = dense[bestI];
    double dist = qSqrt((cp.x() - discCp.x()) * (cp.x() - discCp.x())
                      + (cp.y() - discCp.y()) * (cp.y() - discCp.y()));
    double segLen = qSqrt((pts[1].x() - pts[0].x()) * (pts[1].x() - pts[0].x())
                        + (pts[1].y() - pts[0].y()) * (pts[1].y() - pts[0].y()));
    EXPECT_LT(dist, segLen * 1.5) << "Continuous CP too far from discrete";
}

TEST(SISLCurveTest, rayIntersection_vs_discrete)
{
    QVector<QPointF> pts = { {0, 0}, {1, 0.5}, {2, 0}, {3, 0.5} };
    SISLCurve curve(pts, 3, true);

    QPointF origin(0.5, 2.0);
    QPointF dir(0.2, -1.0);

    double dCont = curve.rayIntersection(origin, dir);

    auto dense = curve.evaluateAll(10000);
    double bestD = 1e18;
    bool foundDisc = false;
    for (int i = 0; i < dense.size() - 1; ++i) {
        QPointF seg = dense[i + 1] - dense[i];
        double denom = dir.x() * seg.y() - dir.y() * seg.x();
        if (qAbs(denom) < 1e-12) continue;
        QPointF diff = dense[i] - origin;
        double tRay = (diff.x() * seg.y() - diff.y() * seg.x()) / denom;
        double tSeg = (diff.x() * dir.y() - diff.y() * dir.x()) / -denom;
        if (tRay >= 0.0 && tSeg >= 0.0 && tSeg <= 1.0 && tRay < bestD) {
            bestD = tRay;
            foundDisc = true;
        }
    }

    if (foundDisc && dCont >= 0.0) {
        EXPECT_NEAR(dCont, bestD, 0.1) << "Continuous ray dist too far from discrete";
    }
}