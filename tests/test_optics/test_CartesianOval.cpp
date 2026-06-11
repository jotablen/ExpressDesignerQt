#include <gtest/gtest.h>
#include <optics/CartesianOval.h>

using namespace ExpressDesigner;

TEST(CartesianOvalTest, DefaultResult)
{
    CartesianOvalCalculator::Result result;
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.errorMessage.isEmpty());
    EXPECT_TRUE(result.ovalPoints.isEmpty());
    EXPECT_DOUBLE_EQ(result.opticalPathLength, 0.0);
}

TEST(CartesianOvalTest, CalculateWithSymmetricInput)
{
    CartesianOvalCalculator::Input input;
    // WF1: horizontal line at y=5
    for (int i = -10; i <= 10; ++i)
        input.wavefront1.append(QPointF(static_cast<double>(i), 5.0));
    input.index1 = 1.0;
    input.isWF1Real = true;

    // WF2: horizontal line at y=-5
    for (int i = -10; i <= 10; ++i)
        input.wavefront2.append(QPointF(static_cast<double>(i), -5.0));
    input.index2 = 1.5;
    input.isWF2Real = true;

    input.referencePoint = QPointF(0.0, 0.0);
    input.numResultPoints = 50;

    auto result = CartesianOvalCalculator::calculate(input);
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.ovalPoints.isEmpty());
    EXPECT_GT(result.opticalPathLength, 0.0);
}

TEST(CartesianOvalTest, CalculateWithEmptyWavefront)
{
    CartesianOvalCalculator::Input input;
    input.index1 = 1.0;
    input.index2 = 1.5;
    input.referencePoint = QPointF(0.0, 0.0);

    auto result = CartesianOvalCalculator::calculate(input);
    EXPECT_FALSE(result.success);
}