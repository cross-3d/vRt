#pragma once

#include "Headers.inl"

// store in official namespace
namespace vrt {

    // system vectors of ray tracers
#pragma pack(push, 1)
    struct VtVec4 { float m[4] = { 0.f,0.f,0.f,0.f }; };
    struct VtVec3 { float m[3] = { 0.f,0.f,0.f }; };
    struct VtVec2 { float m[2] = { 0.f,0.f }; };
    struct VtMat4 { VtVec4 m[4] = { {1.f,0.f,0.f,0.f}, {0.f,1.f,0.f,0.f}, {0.f,0.f,1.f,0.f}, {0.f,0.f,0.f,1.f} }; };
    struct VtMat3x4 { VtVec4 m[3] = { {1.f,0.f,0.f,0.f}, {0.f,1.f,0.f,0.f}, {0.f,0.f,1.f,0.f} }; };
    struct VtUVec2 { uint32_t m[2] = { 0u, 0u }; };
    struct VtUVec4 { uint32_t m[4] = { 0u, 0u, 0u, 0u }; };
    struct VtIVec4 { int32_t m[4] = { 0, 0, 0, 0 }; };
#pragma pack(pop)

    // identified matrix 
    constexpr inline static const VtMat4 IdentifyMat4{ {
        {1.f,0.f,0.f,0.f},
        {0.f,1.f,0.f,0.f},
        {0.f,0.f,1.f,0.f},
        {0.f,0.f,0.f,1.f},
    } };

    // identified matrix 
    constexpr inline static const VtMat3x4 IdentifyMat3x4{ {
        {1.f,0.f,0.f,0.f},
        {0.f,1.f,0.f,0.f},
        {0.f,0.f,1.f,0.f},
    } };

    // 
    //struct VtBufferRegion { VkBuffer buffer = VK_NULL_HANDLE; VkDeviceSize offset = 0ull, range = 4ull; };
    using VtBufferRegion = VkDescriptorBufferInfo;

    // in future planned custom ray structures support
    // in current moment we will using 32-byte standard structuring
#pragma pack(push, 1)
    struct VtRay {
        VtVec3 origin = {}; // position state (in 3D)
        int32_t hitID = 0; // id of intersection hit (-1 is missing)
        VtVec2 cdirect = {}; // polar direction
        uint32_t _indice = 0; // reserved for indice in another ray system
        uint16_t hf_r = 0, hf_g = 0, hf_b = 0, bitfield = 0;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct VtPrimitiveBitfield { uint32_t hitGroup : 2, frontFace : 1, backFace : 1, secondary: 1; };
#pragma pack(pop)

#pragma pack(push, 1)
    struct VtUniformBlock {
        uint32_t primitiveCount = 0;
        uint32_t verticeAccessor = 0;
        uint32_t indiceAccessor = 0xFFFFFFFFu;
        uint32_t materialAccessor = 0xFFFFFFFFu;

        uint32_t primitiveOffset = 0;
        uint32_t attributeOffset = 0;
        uint32_t attributeCount = 8;
        union {
            uint32_t bitfield = 0u;
            VtPrimitiveBitfield bitfieldDetail;
        };

        uint32_t materialID = 0;
        uint32_t readOffset = 0;
        uint32_t reserved0 = 0, reserved1 = 0;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct VtStageUniform { 
        int32_t currentGroup = 0, maxRayCount = 0, maxHitCount = 0, closestHitOffset = 0;
        int32_t width = 1, height = 1, lastIteration = 0, iteration = 0;
    };
#pragma pack(pop)

    // reduced to 4x6(x4) or 96 bytes
#pragma pack(push, 1)
    struct VtBvhBlock {
        int32_t entryID = 0u, leafCount = 0u, primitiveCount = 0u, primitiveOffset = 0u;
        VtMat3x4 transform = IdentifyMat3x4;//, transformInv = IdentifyMat4;
        VtVec4 sceneMin = {}, sceneMax = {};
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct VtBvhInstance {
        VtMat3x4 //transform   = IdentifyMat3x4,
            transformIn = IdentifyMat3x4; // combined transform 
        int32_t bvhBlockID = 0u, bvhVariationID = -1, r0 = 0u, r1 = 0u;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct VtBuildConst {
        int32_t primitiveCount = 0u, primitiveOffset = 0u;
    };
#pragma pack(pop)

    // required for measuring required bytes and debugging 
#pragma pack(push, 1)
    struct VtBvhNodeStruct {
        VtUVec2 ch0 = {}, ch1 = {}, ch2 = {}, ch3 = {}, ch4 = {}, ch5 = {};
        VtUVec4 data = {};
    };
#pragma pack(pop)


    // helping to calculate possible entry ID by BVH data byte offset
    inline static uint32_t VtMeasureEntryIDByByteOffset(VkDeviceSize byteOffset = 0ull) {
        return uint32_t(((byteOffset / sizeof(VtBvhNodeStruct) + 1ull) >> 1ull) << 1ull);
    };

    // helping to calculate correct offset by entry ID 
    inline static VkDeviceSize VtMeasureByteOffsetByEntryID(uint32_t entryID = 0ull) {
        return ((entryID * sizeof(VtBvhNodeStruct)) << 1ull) >> 1ull;
    };


#pragma pack(push, 1)
    class uint24_t;
#pragma pack(pop)

#pragma pack(push, 1)
    struct uint24_p { uint8_t _data[3] = { 0u, 0u, 0u }; };
#pragma pack(pop)

#pragma pack(push, 1)
    class uint24_u {
    private:
        union { uint24_p _part; uint32_t _data : 24; };
    protected:
        uint24_u(const uint24_p& _input) : _part(_input) {};
        operator const uint24_p&() const { return _part; };
    public: friend uint24_t;
            uint24_u(const uint32_t& _input = 0u) : _data(_input) {};
            operator uint32_t() const { return _data; };
    };
#pragma pack(pop)

#pragma pack(push, 1)
    class uint24_t {
    private:
        uint24_p _data;
    public:
        uint24_t(const uint32_t& _input = 0u) : _data(uint24_u(_input)) {};
        operator uint32_t() const { return uint24_u(_data); };
    };
#pragma pack(pop)



};
