#ifndef _DRIVER_H
#define _DRIVER_H

// disable RX Vega functions in other platforms
#ifndef AMD_PLATFORM
#undef ENABLE_VEGA_INSTRUCTION_SET
#endif

// disable NVidia Turing functions in other platforms
#ifndef NVIDIA_PLATFORM
#undef ENABLE_TURING_INSTRUCTION_SET
#endif

// AMuDe extensions
#ifdef ENABLE_VEGA_INSTRUCTION_SET
#extension GL_AMD_shader_trinary_minmax : enable
#extension GL_AMD_texture_gather_bias_lod : enable
#extension GL_AMD_shader_image_load_store_lod : enable
#extension GL_AMD_gcn_shader : enable
#endif

// 
//#extension GL_KHR_shader_atomic_int64 : enable
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_EXT_shader_8bit_storage : enable
#extension GL_EXT_control_flow_attributes : enable
#extension GL_EXT_shader_image_load_formatted : enable
#extension GL_KHR_memory_scope_semantics : enable

// subgroup operations
#extension GL_KHR_shader_subgroup_basic            : require
#extension GL_KHR_shader_subgroup_vote             : require
#extension GL_KHR_shader_subgroup_ballot           : require
#extension GL_KHR_shader_subgroup_arithmetic       : enable
#extension GL_KHR_shader_subgroup_shuffle          : enable
#extension GL_KHR_shader_subgroup_shuffle_relative : enable
#extension GL_KHR_shader_subgroup_clustered        : enable

// 
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_samplerless_texture_functions : enable
//#extension GL_EXT_subgroupuniform_qualifier : enable

// if Vega 10 specific
#ifdef ENABLE_VEGA_INSTRUCTION_SET
    #define ENABLE_INT16_SUPPORT
    #define ENABLE_FP16_SUPPORT
#endif

// if Turing specific
#ifdef ENABLE_TURING_INSTRUCTION_SET
    #define ENABLE_INT16_SUPPORT
    #define ENABLE_FP16_SUPPORT
    #extension GL_NV_shader_subgroup_partitioned : require // volta and above should support it
    #extension GL_NV_shader_atomic_int64 : enable // unknown status
#endif

// enable fp16 support
#ifdef ENABLE_FP16_SUPPORT
    #extension GL_AMD_gpu_shader_half_float : enable
    #extension GL_AMD_gpu_shader_half_float_fetch : enable
    //#extension GL_KHR_gpu_shader_float16 : enable // unknown status
#endif

// enable int16 support
#ifdef ENABLE_INT16_SUPPORT
    #extension GL_AMD_gpu_shader_int16 : enable
#endif

// if int16 no supported, use plain int32
#ifndef ENABLE_INT16_SUPPORT
    #undef USE_INT16_FOR_MORTON
#endif

// enable 16-bit strictly
#if (defined(ENABLE_FP16_SUPPORT) || defined(ENABLE_INT16_SUPPORT))
#extension GL_KHX_shader_explicit_arithmetic_types : require
#extension GL_EXT_shader_16bit_storage : require
#else
#extension GL_KHX_shader_explicit_arithmetic_types : enable
#extension GL_EXT_shader_16bit_storage : enable
#endif

// platform-oriented compute
#ifdef EXTEND_LOCAL_GROUPS
#ifdef ENABLE_VEGA_INSTRUCTION_SET
    #define WORK_SIZE 512u
#else
    #define WORK_SIZE 768u
#endif
#endif

#ifndef WORK_SIZE
    #define WORK_SIZE 64u
#endif

#define LOCAL_SIZE_LAYOUT layout(local_size_x=WORK_SIZE)in
#define USE_MORTON_32

#ifndef WORK_SIZE_BND
#define WORK_SIZE_BND WORK_SIZE
#endif

// packing as unorm16x2 (experimental)
#define EXPERIMENTAL_UNORM16_DIRECTION
#define EXPERIMENTAL_UNORM16_BVH

#endif
