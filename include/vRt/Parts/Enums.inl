#pragma once
#include "Headers.inl"

namespace vt { // store in official namespace

    // for not confusing with Vulkan API
    // use 0x11E for VtResult
    // use 0x11F for VtStructureType

    typedef enum VtResult: uint32_t {
        VT_SUCCESS = 0x11E00000, // default status
        VT_NOT_READY = 0x11E00001, 
        VT_TIMEOUT = 0x11E00002, 
        VT_INCOMPLETE = 0x11E00003, 
        VT_ERROR_INITIALIZATION_FAILED = 0x11E00004, // if error occurs from ray tracer itself
    } VtResult;

    typedef enum VtPipelineBindPoint: uint32_t {
        VT_PIPELINE_BIND_POINT_RAY_TRACING = 0x11F00000,
        VT_PIPELINE_BIND_POINT_ACCELERATOR = 0x11F00001 // unknown
    } VtPipelineBindPoint;

    typedef enum VtStructureType: uint32_t {
        VT_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 0x11F00000,
        VT_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 0x11F00001,
        VT_STRUCTURE_TYPE_DEVICE_CONVERT_INFO = 0x11F00002, 
        VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERT_INFO = 0x11F00003,
        VT_STRUCTURE_TYPE_RAY_TRACING_CREATE_INFO = 0x11F00004,
        VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO = 0x11F00005,
        VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO = 0x11F00006,
        VT_STRUCTURE_TYPE_DEVICE_BUFFER_CREATE_INFO = 0x11F00007,
        VT_STRUCTURE_TYPE_DEVICE_IMAGE_CREATE_INFO = 0x11F00008,
        // empty
        VT_STRUCTURE_TYPE_MATERIALS_INPUT_CREATE_INFO = 0x11F0000A,
        VT_STRUCTURE_TYPE_ACCELERATOR_CREATE_INFO = 0x11F0000B
    } VtStructureType;

    // it is bitfield-based value
    typedef enum VtFormat: uint32_t {

    } VtFormat;

    // all supported topologies
    typedef enum VtTopologyType: uint32_t {
        VT_TOPOLOGY_TYPE_TRIANGLES_LIST = 0x11F00000
    } VtTopologyType;

};