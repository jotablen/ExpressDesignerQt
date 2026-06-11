#include <gtest/gtest.h>
#include <optics/CartesianOval.h>
#include <optics/WavefrontPropagator.h>
#include <optics/SnellLaw.h>
#include <optics/OpticalPathLength.h>

#include <QtMath>

using namespace ExpressDesigner;
using namespace ExpressDesigner::Optics;

/**
 * @brief Integration test: Cartesian oval coupling a point WF1 to a plane WF2.
 *
 * WF1:  point source at (1,1), n=1.5
 * WF2:  plane along the line from (6,-6) to (6,7), n=1.0
 * Reference point: (3,1)
 *
 * Expected result:
 *   - The CartesianOvalCalculator finds a valid oval surface.
 *   - Propagating WF1 rays through this surface yields a wavefront
 *     whose normals are parallel (i.e., it becomes a plane wave).
 */
TEST(OvalCouplingTest, PointToPlaneCoupling)
{
    // --- Step 1: Build the CartesianOvalCalculator input ---
    CartesianOvalCalculator::Input ovalInput;

    // WF1: point source at (1,1). Represent as a tiny spread around the point
    // so the calculator has enough points to work with.
    for (int i = 0; i < 10; ++i)
        ovalInput.wavefront1.append(QPointF(1.0 + (i - 5) * 0.02, 1.0));
    ovalInput.index1 = 1.5;
    ovalInput.isWF1Real = true;

    // WF2: plane at x=6, from y=-6 to y=7
    for (int i = 0; i <= 20; ++i) {
        double y = -6.0 + i * (13.0 / 20.0); // -6 to 7
        ovalInput.wavefront2.append(QPointF(6.0, y));
    }
    ovalInput.index2 = 1.0;
    ovalInput.isWF2Real = true;

    ovalInput.referencePoint = QPointF(3.0, 1.0);
    ovalInput.numResultPoints = 80;

    // --- Step 2: Calculate the Cartesian oval ---
    auto ovalResult = CartesianOvalCalculator::calculate(ovalInput);
    ASSERT_TRUE(ovalResult.success) << "Oval calculation failed: "
                                    << ovalResult.errorMessage.toStdString();
    ASSERT_GE(ovalResult.ovalPoints.size(), 3u);
    EXPECT_GT(ovalResult.opticalPathLength, 0.0);

    // --- Step 3: Propagate WF1 through the oval surface ---
    // Use the oval points as the refracting surface
    WavefrontPropagator::Input propInput;
    propInput.wavefront = ovalInput.wavefront1;  // same source WF
    propInput.inputIndex = ovalInput.index1;
    propInput.refractingSurface = ovalResult.ovalPoints;
    propInput.outputIndex = ovalInput.index2;
    propInput.numResultPoints = ovalInput.numResultPoints;
    propInput.excludeTIR = false;
    propInput.offset = 0.0;

    auto propResult = WavefrontPropagator::calculate(propInput);
    ASSERT_TRUE(propResult.success) << "Propagation failed: "
                                    << propResult.errorMessage.toStdString();
    ASSERT_GE(propResult.propagatedWF.size(), 3u);

    // --- Step 4: Verify that the propagated WF has parallel normals ---
    // Compute direction vectors between consecutive points of the propagated WF
    QVector<QPointF> directions;
    for (int i = 1; i < propResult.propagatedWF.size(); ++i) {
        QPointF delta = propResult.propagatedWF[i] - propResult.propagatedWF[i - 1];
        double len = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
        if (len > 1e-9)
            directions.append(delta / len);
    }
    ASSERT_GE(directions.size(), 2u);

    // The normals to the wavefront are perpendicular to the tangent directions.
    // If normals are parallel, the tangents are parallel too.
    // Check dot product between first and all other directions.
    QPointF refDir = directions.first();
    for (int i = 1; i < directions.size(); ++i) {
        double dot = refDir.x() * directions[i].x() + refDir.y() * directions[i].y();
        // Directions should be nearly parallel → |dot| ≈ 1.0
        EXPECT_NEAR(qAbs(dot), 1.0, 0.15)
            << "Direction " << i << " deviates from parallel (dot=" << dot << ")";
    }
}

/**
 * @brief Verifies that the OPL computed by the oval matches the OPL along
 *        the actual ray paths through the surface.
 */
TEST(OvalCouplingTest, OpticalPathLengthConsistency)
{
    CartesianOvalCalculator::Input ovalInput;

    // Simple case: point to point
    ovalInput.wavefront1.append(QPointF(0.0, 5.0));
    ovalInput.index1 = 1.0;
    ovalInput.isWF1Real = true;

    ovalInput.wavefront2.append(QPointF(0.0, -5.0));
    ovalInput.index2 = 1.5;
    ovalInput.isWF2Real = true;

    ovalInput.referencePoint = QPointF(0.0, 0.0);
    ovalInput.numResultPoints = 50;

    auto ovalResult = CartesianOvalCalculator::calculate(ovalInput);
    ASSERT_TRUE(ovalResult.success);

    // The OPL should be positive and consistent
    // OPL = n1 * d1 + n2 * d2 = 1.0 * sqrt((0-0)^2 + (5-0)^2) + 1.5 * sqrt((0-0)^2 + (-5-0)^2)
    // = 5 + 7.5 = 12.5
    double expectedOPL = OpticalPathLength::compute(QPointF(0.0, 5.0),
                                                     QPointF(0.0, 0.0), 1.0)
                       + OpticalPathLength::compute(QPointF(0.0, 0.0),
                                                     QPointF(0.0, -5.0), 1.5);
    EXPECT_NEAR(ovalResult.opticalPathLength, expectedOPL, 1.0);
}