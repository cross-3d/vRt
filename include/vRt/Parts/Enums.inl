#pragma once
#include "Vulkan.inl"

namespace vt { // store in official namespace

    // for not confusing with Vulkan API
    // use 0x11E for VtResult
    // use 0x11F for VtStructureType

     enum VtResult: uint32_t {
        VT_SUCCESS = 0x11E00000, // default status
        VT_NOT_READY = 0x11E00001, 
        VT_TIMEOUT = 0x11E00002, 
        VT_INCOMPLETE = 0x11E00003, 
        VT_ERROR_INITIALIZATION_FAILED = 0x11E00004, // if error occurs from ray tracer itself
    };

     enum VtPipelineBindPoint: uint32_t {
        VT_PIPELINE_BIND_POINT_RAY_TRACING = 0x11F00000,
        VT_PIPELINE_BIND_POINT_ACCELERATOR = 0x11F00001 // unknown
    };

     enum VtStructureType: uint32_t {
        VT_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 0x11F00000,
        VT_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 0x11F00001,
        VT_STRUCTURE_TYPE_DEVICE_CONVERT_INFO = 0x11F00002, 
        VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERT_INFO = 0x11F00003,
        VT_STRUCTURE_TYPE_RAY_TRACING_CREATE_INFO = 0x11F00004,
        VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO = 0x11F00005,
        VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO = 0x11F00006,
        //VT_STRUCTURE_TYPE_VERTEX_ACCESSOR = 0x11F00007,
        //VT_STRUCTURE_TYPE_VERTEX_ATTRIBUTE_BINDING = 0x11F00008,
        //VT_STRUCTURE_TYPE_VERTEX_REGION_BINDING = 0x11F00009,
        VT_STRUCTURE_TYPE_MATERIALS_INPUT_CREATE_SET_INFO = 0x11F0000A,
    };

    // it is bitfield-based value
     enum VtFormat: uint32_t {

    };

    // all supported topologies
     enum VtTopologyType: uint32_t {
        VT_TOPOLOGY_TYPE_TRIANGLES_LIST = 0x11F00000
    };

};