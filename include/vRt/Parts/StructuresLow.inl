#pragma once
#include "Headers.inl"
//#include "HardClassesDef.inl"
//#include "HandlersDef.inl"
//#include "Enums.inl"

namespace vt { // store in official namespace



    // any other vertex bindings can be used by attributes
    struct VtVertexRegionBinding {
        uint32_t byteOffset = 0;
        uint32_t byteSize = 0;
    };

    // buffer view
    struct VtVertexBufferView {
        uint32_t regionID = 0;
        uint32_t byteOffset = 0;
        uint32_t byteStride = 0;
    };

    // attribute binding
    struct VtVertexAttributeBinding {
        uint32_t attributeBinding = 0;
        uint32_t accessorID = 0;
    };

    // system vectors of ray tracers
    struct VtVec4 { float x, y, z, w; };
    struct VtVec3 { float x, y, z; };
    struct VtVec2 { float x, y; };
    struct VtUVec2 { uint32_t x, y; };

    // in future planned custom ray structures support
    // in current moment we will using 32-byte standard structuring
    struct VtRay {
        VtVec3 origin; // position state (in 3D)
        int32_t hitID; // id of intersection hit (-1 is missing)
        VtVec2 cdirect; // polar direction
        uint32_t _indice; // reserved for indice in another ray system
        uint16_t hf_r, hf_g, hf_b, bitfield;
    };

    struct VtVirtualCombinedImage {
        union {
            uint64_t textureID : 32, samplerID : 32;
            uint64_t combined = 0ull;
        };

        operator uint64_t() const { return combined; };
        operator uint64_t&() { return combined; };
    };

    struct VtVertexDataBufferBinding {
        uint32_t binding = 0;
        VkBuffer pBuffer = nullptr;
        VkDeviceSize offset = 0;
    };



    typedef enum VtType : uint32_t {
        VT_FLOAT = 0,
        VT_UINT32 = 1,
        VT_UINT16 = 2,
        VT_HALF = 3
    } VtType;

    
    struct VtFormatDecomp {
        union {
            uint32_t components : 2, type : 4, normalized : 1;
            uint32_t format = 0;
        };
        constexpr VtFormatDecomp() : components(0), type(0), normalized(0) {};
        constexpr VtFormatDecomp(uint32_t _components, uint32_t _type, uint32_t _normalized = 0) : components(_components), type(_type), normalized(_normalized) {};
        constexpr VtFormatDecomp(uint32_t _format) : format(_format) {};
        operator uint32_t() const { return format; }
    };

    // any other vertex accessors can be used by attributes
    struct VtVertexAccessor {
        uint32_t bufferViewID = 0;
        uint32_t byteOffset = 0;
        VtFormatDecomp format;
    };


};
