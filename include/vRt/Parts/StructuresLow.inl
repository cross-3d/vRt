#pragma once

#include "Headers.inl"


// store in official namespace
namespace vrt {

    // system vectors of ray tracers
    struct VtVec4 { float x, y, z, w; };
    struct VtVec3 { float x, y, z; };
    struct VtVec2 { float x, y; };
    struct VtUVec2 { uint32_t x, y; };
    struct VtMat4 { VtVec4 m0, m1, m2, m3; };

    // in future planned custom ray structures support
    // in current moment we will using 32-byte standard structuring
    struct VtRay {
        VtVec3 origin; // position state (in 3D)
        int32_t hitID; // id of intersection hit (-1 is missing)
        VtVec2 cdirect; // polar direction
        uint32_t _indice; // reserved for indice in another ray system
        uint16_t hf_r, hf_g, hf_b, bitfield;
    };

    struct VtPrimitiveBitfield { uint32_t hitGroup : 2, frontFace : 1, backFace : 1; };

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
        VtMat4 transform;
        VtMat4 transformInv;
        VtMat4 projection;
        VtMat4 projectionInv;
        int leafCount = 0, primitiveCount = 0, entryID = 0, primitiveOffset = 0;
    };

};
