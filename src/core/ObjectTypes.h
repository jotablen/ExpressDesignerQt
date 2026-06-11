#pragma once

#include <cstdint>
#include <QUuid>

namespace ExpressDesigner {

enum class ObjectType : uint16_t {
    Point       = 0x001,
    Line        = 0x002,
    Arc         = 0x003,
    Curve       = 0x004,
    Mask_WF         = 0x100,
    Mask_VirtualWF  = 0x400,
    Mask_Result     = 0x200,
    Mask_Project    = 0x800,
};

enum class OperationType : uint16_t {
    Custom        = 0x010,
    PropagateWF   = 0x011,
    CartesianOval = 0x012,
};

enum class ObjectListType {
    Data   = 0x1,
    Result = 0x2,
};

inline constexpr bool isWavefront(ObjectType type) {
    return (static_cast<uint16_t>(type) & static_cast<uint16_t>(ObjectType::Mask_WF)) != 0;
}

inline constexpr bool isVirtualWF(ObjectType type) {
    auto v = static_cast<uint16_t>(type);
    auto mask = static_cast<uint16_t>(ObjectType::Mask_WF) | static_cast<uint16_t>(ObjectType::Mask_VirtualWF);
    return (v & mask) == mask;
}

inline constexpr bool isResult(ObjectType type) {
    return (static_cast<uint16_t>(type) & static_cast<uint16_t>(ObjectType::Mask_Result)) != 0;
}

inline constexpr uint16_t toBaseType(ObjectType type) {
    return static_cast<uint16_t>(type) & 0x0FF;
}

inline constexpr ObjectType withWavefront(ObjectType type) {
    return static_cast<ObjectType>(static_cast<uint16_t>(type) | static_cast<uint16_t>(ObjectType::Mask_WF));
}

inline constexpr ObjectType withVirtualWF(ObjectType type) {
    auto v = static_cast<uint16_t>(type);
    v |= static_cast<uint16_t>(ObjectType::Mask_WF);
    v |= static_cast<uint16_t>(ObjectType::Mask_VirtualWF);
    return static_cast<ObjectType>(v);
}

inline constexpr ObjectType withResult(ObjectType type) {
    return static_cast<ObjectType>(static_cast<uint16_t>(type) | static_cast<uint16_t>(ObjectType::Mask_Result));
}

} // namespace ExpressDesigner