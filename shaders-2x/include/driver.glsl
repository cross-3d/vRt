#ifndef _DRIVER_H
#define _DRIVER_H


// AMuDe extensions
#ifdef ENABLE_AMD_INSTRUCTION_SET
#extension GL_AMD_shader_trinary_minmax : enable
#extension GL_AMD_texture_gather_bias_lod : enable
#extension GL_AMD_shader_image_load_store_lod : enable
#extension GL_AMD_gcn_shader : enable
#extension GL_AMD_gpu_shader_half_float : enable
#extension GL_AMD_gpu_shader_int16 : enable
#endif

// ARB and ext
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_EXT_control_flow_attributes : enable
#extension GL_EXT_shader_image_load_formatted : enable

// subgroup operations
#extension GL_KHR_shader_subgroup_basic            : enable
#extension GL_KHR_shader_subgroup_vote             : enable
#extension GL_KHR_shader_subgroup_arithmetic       : enable
#extension GL_KHR_shader_subgroup_ballot           : enable
#extension GL_KHR_shader_subgroup_shuffle          : enable
#extension GL_KHR_shader_subgroup_shuffle_relative : enable
#extension GL_KHR_shader_subgroup_clustered        : enable

// non uniform (for bindless textures)
#extension GL_EXT_nonuniform_qualifier : enable


// ray tracing options
//#define EXPERIMENTAL_DOF // no dynamic control supported
#define ENABLE_PT_SUNLIGHT
#define DIRECT_LIGHT_ENABLED

//#define SIMPLE_RT_MODE
//#define USE_TRUE_METHOD
//#define DISABLE_REFLECTIONS

// sampling options
//#define MOTION_BLUR
#ifndef SAMPLES_LOCK
#define SAMPLES_LOCK 1
#endif

// disable AMD functions in other platforms
#ifndef AMD_PLATFORM
#undef ENABLE_AMD_INSTRUCTION_SET
#endif

// enable required GAPI extensions
#ifdef ENABLE_AMD_INSTRUCTION_SET
    #define ENABLE_AMD_INT16 // RX Vega broken support 16-bit integer buffers in Vulkan API 1.1.70
    #define ENABLE_AMD_INT16_CONDITION
    #define USE_16BIT_ADDRESS_SPACE
#endif

#ifndef ENABLE_AMD_INSTRUCTION_SET
    #undef ENABLE_AMD_INT16 // not supported combination
#endif

#ifndef ENABLE_AMD_INT16
    #undef ENABLE_AMD_INT16_CONDITION // required i16
#endif

// Platform-oriented compute
#ifndef WORK_SIZE
    #ifdef NVIDIA_PLATFORM
        #define WORK_SIZE 32
    #else
        #define WORK_SIZE 64
    #endif
#endif

#define LOCAL_SIZE_LAYOUT layout(local_size_x=WORK_SIZE)in

#endif
