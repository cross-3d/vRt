#pragma once

#include "Headers.inl"


// store in official namespace
namespace vrt {

    // system vectors of ray tracers
    struct VtVec4 { float x = 0.f, y = 0.f, z = 0.f, w = 0.f; };
    struct VtVec3 { float x = 0.f, y = 0.f, z = 0.f; };
    struct VtVec2 { float x = 0.f, y = 0.f; };
    struct VtUVec2 { uint32_t x = 0u, y = 0u; };
    struct VtMat4 { VtVec4 m0, m1, m2, m3; };

    // identified matrix 
    constexpr inline static const VtMat4 IdentifyMat4 = {
        {1.f,0.f,0.f,0.f},
        {0.f,1.f,0.f,0.f},
        {0.f,0.f,1.f,0.f},
        {0.f,0.f,0.f,1.f},
    };


    // in future planned custom ray structures support
    // in current moment we will using 32-byte standard structuring
    struct VtRay {
        VtVec3 origin; // position state (in 3D)
        int32_t hitID; // id of intersection hit (-1 is missing)
        VtVec2 cdirect; // polar direction
        uint32_t _indice; // reserved for indice in another ray system
        uint16_t hf_r, hf_g, hf_b, bitfield;
    };

    struct VtPrimitiveBitfield { uint32_t hitGroup : 2, frontFace : 1, backFace : 1, secondary: 1; };

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

    struct VtStageUniform { 
        int currentGroup = 0, maxRayCount = 0, iteration = 0, closestHitOffset = 0;
        int width = 1, height = 1, lastIteration = 0, r0 = 0;
    };

    struct VtBvhBlock {
        VtMat4 transform = IdentifyMat4;
        VtMat4 transformInv = IdentifyMat4;
        VtMat4 projection = IdentifyMat4;
        VtMat4 projectionInv = IdentifyMat4;
        int leafCount = 0, primitiveCount = 0, entryID = 0, primitiveOffset = 0;
        VtVec4 sceneMin, sceneMax;
    };

};
