#pragma once

#include "StructuresLow.inl"

namespace vrt { // store in official namespace

    // in general that is conversion
    struct VtInstanceConversionInfo;
    struct VtArtificalDeviceExtension;
    //struct VtDeviceConversionInfo;
    //struct VtPhysicalDeviceConversionInfo;
    struct VtRayTracingPipelineCreateInfo;

    // use immutables in accelerator inputs
    struct VtVertexInputCreateInfo;

    // use as low level typed descriptor set
    struct VtMaterialSetCreateInfo;
    struct VtAcceleratorSetCreateInfo;

    // custom (unified) object create info, exclusive for vRt ray tracing system, and based on classic Satellite objects
    // bound in device space
    struct VtDeviceBufferCreateInfo;
    struct VtDeviceImageCreateInfo;
    struct VtBufferRegionCreateInfo;
    struct VtPipelineLayoutCreateInfo;
    struct VtAttributePipelineCreateInfo;
    struct VtRayTracingSetCreateInfo;
    struct VtVertexAssemblySetCreateInfo;

    // advanced acceleration extension
    class VtDeviceAdvancedAccelerationExtension;

    // constexpr format compositor
    struct VtFormatDecomp {
        union { uint32_t _format = 0; struct { uint32_t _components : 2, _type : 4, _normalized : 1; } _formatDecomp; };

        constexpr VtFormatDecomp() {};
        constexpr VtFormatDecomp(uint8_t components, uint8_t type, uint8_t normalized = 0) : _formatDecomp({ components - 1u , type, normalized }) {};
        constexpr VtFormatDecomp(uint32_t format) : _format(format) {};
        constexpr operator uint32_t() const { return _format; };
        //operator uint32_t() const { return _format; };

        constexpr VtFormatDecomp& setComponents(uint8_t components) { _formatDecomp._components = components - 1u; return *this; };
        constexpr VtFormatDecomp& setType(uint8_t type) { _formatDecomp._type = type; return *this; };
        constexpr VtFormatDecomp& setNormalized(bool normalized) { _formatDecomp._normalized = normalized; return *this; };

        constexpr uint8_t getComponents() const { return _formatDecomp._components + 1u; };
        constexpr uint8_t getType() const { return _formatDecomp._type; };
        constexpr bool getNormalized() const { return _formatDecomp._normalized; };
    };

    struct VtVirtualCombinedImage {
        union {
            uint64_t _combined = 0ull;
            struct { uint64_t textureID : 32, samplerID : 32; } _combination;
        };

        // per-component constructor
        VtVirtualCombinedImage(uint32_t textureID = 0u, uint32_t samplerID = 0u) : _combination({ textureID + 1u, samplerID + 1u }) {};
        VtVirtualCombinedImage(uint64_t combined) : _combined(combined) {};
        //VtVirtualCombinedImage() {};

        // component setters
        VtVirtualCombinedImage& setTextureID(uint32_t textureID = 0) { _combination.textureID = textureID + 1u; return *this; }
        VtVirtualCombinedImage& setSamplerID(uint32_t samplerID = 0) { _combination.samplerID = samplerID + 1u; return *this; }

        // component getters
        uint32_t getTextureID() const { return uint32_t(_combination.textureID) - 1u; }
        uint32_t getSamplerID() const { return uint32_t(_combination.samplerID) - 1u; }

        // casting operator
        operator uint64_t() const { return _combined; };
        operator uint64_t&() { return _combined; };
    };

    typedef enum VtType : uint32_t {
        VT_TYPE_FLOAT = 0,
        VT_TYPE_UINT32 = 1,
        VT_TYPE_UINT16 = 2,
        VT_TYPE_HALF = 3
    } VtType;

    // any other vertex accessors can be used by attributes
    struct VtVertexAccessor {
        uint32_t bufferViewID = 0;
        uint32_t byteOffset = 0;
        VtFormatDecomp format;
        uint32_t reserved = 0;
    };

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
        uint32_t byteLength = 0;
    };

    // attribute binding
    struct VtVertexAttributeBinding {
        uint32_t attributeBinding = 0;
        uint32_t accessorID = 0;
    };

    // retype VtFormatDecomp
    using VtFormat = VtFormatDecomp;

};
