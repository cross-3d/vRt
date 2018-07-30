#pragma once
#include "Headers.inl"
#include "StructuresLow.inl"

namespace vrt { // store in official namespace
    // for not confusing with Vulkan API enums
    // use 0x11E for VtResult
    // use 0x11F for VtStructureType

    // planned merge ray tracing exclusive errors
    typedef VkResult VtResult;

    typedef enum VtPipelineBindPoint : uint32_t {
        VT_PIPELINE_BIND_POINT_RAYTRACING = 0x11F00000,
        VT_PIPELINE_BIND_POINT_VERTEXASSEMBLY = 0x11F00001
    } VtPipelineBindPoint;

    typedef enum VtStructureType : uint32_t {
        VT_STRUCTURE_TYPE_INSTANCE_CONVERSION_INFO = 0x11F00000,
        VT_STRUCTURE_TYPE_DEVICE_CONVERSION_INFO = 0x11F00001,
        VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERSION_INFO = 0x11F00002,
        VT_STRUCTURE_TYPE_RAY_TRACING_CREATE_INFO = 0x11F00003,
        VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO = 0x11F00004,
        VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO = 0x11F00005,
        VT_STRUCTURE_TYPE_DEVICE_BUFFER_CREATE_INFO = 0x11F00006,
        VT_STRUCTURE_TYPE_DEVICE_IMAGE_CREATE_INFO = 0x11F00007,
        VT_STRUCTURE_TYPE_DEVICE_TO_HOST_BUFFER_CREATE_INFO = 0x11F00008,
        VT_STRUCTURE_TYPE_DEVICE_TO_HOST_IMAGE_CREATE_INFO = 0x11F00009,
        VT_STRUCTURE_TYPE_HOST_TO_DEVICE_BUFFER_CREATE_INFO = 0x11F0000A,
        VT_STRUCTURE_TYPE_HOST_TO_DEVICE_IMAGE_CREATE_INFO = 0x11F0000B,
        VT_STRUCTURE_TYPE_MATERIAL_SET_CREATE_INFO = 0x11F0000C,
        VT_STRUCTURE_TYPE_ACCELERATOR_SET_CREATE_INFO = 0x11F0000D,
        VT_STRUCTURE_TYPE_VERTEX_ASSEMBLY_SET_CREATE_INFO = 0x11F0000E,
        VT_STRUCTURE_TYPE_ARTIFICAL_DEVICE_EXTENSION = 0x11F0000F,
        VT_STRUCTURE_TYPE_RAY_TRACING_SET_CREATE_INFO = 0x11F00010,
        VT_STRUCTURE_TYPE_VERTEX_ASSEMBLY_PIPELINE_CREATE_INFO = 0x11F00011,
        VT_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO = 0x11F00012
    } VtStructureType;

    // retype VtFormatDecomp
    using VtFormat = VtFormatDecomp;

    // use constexpr VtFormat constants
    constexpr auto
        VT_FORMAT_R32G32B32A32_SFLOAT = VtFormatDecomp(4u, VT_TYPE_FLOAT),
        VT_FORMAT_R32G32B32_SFLOAT = VtFormatDecomp(3u, VT_TYPE_FLOAT),
        VT_FORMAT_R32G32_SFLOAT = VtFormatDecomp(2u, VT_TYPE_FLOAT),
        VT_FORMAT_R32_SFLOAT = VtFormatDecomp(1u, VT_TYPE_FLOAT),
        VT_FORMAT_R32_UINT = VtFormatDecomp(1u, VT_TYPE_UINT32),
        VT_FORMAT_R16_UINT = VtFormatDecomp(1u, VT_TYPE_UINT16);

    // all supported topologies
    typedef enum VtTopologyType : uint32_t {
        VT_TOPOLOGY_TYPE_TRIANGLES_LIST = 0x00000000
    } VtTopologyType;

    // ray tracing pipeline usages
    typedef enum VtEntryUsageFlagBits : uint32_t {
        VT_ENTRY_USAGE_CLOSEST = 0x00000001,
        VT_ENTRY_USAGE_MISS = 0x00000002,
        VT_ENTRY_USAGE_GENERATION = 0x00000004,
        VT_ENTRY_USAGE_GROUP = 0x00000008,
    } VtEntryUsageFlags;

    typedef enum VtPipelineLayoutType : uint32_t {
        VT_PIPELINE_LAYOUT_TYPE_RAYTRACING,
        VT_PIPELINE_LAYOUT_TYPE_VERTEXINPUT
    } VtPipelineLayoutType;


    typedef enum VtVendor : uint32_t {
        VT_VENDOR_UNIVERSAL = 0,
        VT_VENDOR_AMD,
        VT_VENDOR_NVIDIA,
        VT_VENDOR_INTEL
    } VtVendor;
};
