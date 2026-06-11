#include <gtest/gtest.h>
#include <core/ObjectTypes.h>

using namespace ExpressDesigner;

TEST(ObjectTypesTest, BaseTypeValues)
{
    EXPECT_EQ(static_cast<uint16_t>(ObjectType::Point), 0x001);
    EXPECT_EQ(static_cast<uint16_t>(ObjectType::Line), 0x002);
    EXPECT_EQ(static_cast<uint16_t>(ObjectType::Arc), 0x003);
    EXPECT_EQ(static_cast<uint16_t>(ObjectType::Curve), 0x004);
}

TEST(ObjectTypesTest, MaskValues)
{
    EXPECT_EQ(static_cast<uint16_t>(ObjectType::Mask_WF), 0x100);
    EXPECT_EQ(static_cast<uint16_t>(ObjectType::Mask_VirtualWF), 0x400);
    EXPECT_EQ(static_cast<uint16_t>(ObjectType::Mask_Result), 0x200);
    EXPECT_EQ(static_cast<uint16_t>(ObjectType::Mask_Project), 0x800);
}

TEST(ObjectTypesTest, IsWavefront)
{
    EXPECT_TRUE(isWavefront(withWavefront(ObjectType::Curve)));
    EXPECT_FALSE(isWavefront(ObjectType::Curve));
}

TEST(ObjectTypesTest, ToBaseType)
{
    auto wf = withWavefront(ObjectType::Point);
    EXPECT_EQ(toBaseType(wf), 0x001u);
}

TEST(ObjectTypesTest, OperationTypes)
{
    EXPECT_EQ(static_cast<uint16_t>(OperationType::CartesianOval), 0x012);
    EXPECT_EQ(static_cast<uint16_t>(OperationType::PropagateWF), 0x011);
    EXPECT_EQ(static_cast<uint16_t>(OperationType::Custom), 0x010);
}