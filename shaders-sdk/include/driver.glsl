#ifndef _DRIVER_H
#define _DRIVER_H

// disable RX Vega functions in other platforms
#ifndef AMD_PLATFORM
#undef ENABLE_VEGA_INSTRUCTION_SET
#endif

// AMuDe extensions
#ifdef ENABLE_VEGA_INSTRUCTION_SET
#extension GL_AMD_shader_trinary_minmax : enable
#extension GL_AMD_texture_gather_bias_lod : enable
#extension GL_AMD_shader_image_load_store_lod : enable
#extension GL_AMD_gcn_shader : enable
#endif

// ARB and EXT
#ifdef AMD_PLATFORM
#extension GL_KHX_shader_explicit_arithmetic_types : require
#else
#extension GL_KHX_shader_explicit_arithmetic_types : enable
#endif

// 
#extension GL_ARB_gpu_shader_int64 : require
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

// texture extensions
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_samplerless_texture_functions : enable


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

// enable required GAPI extensions
#ifdef ENABLE_VEGA_INSTRUCTION_SET
    #define ENABLE_INT16_SUPPORT
    #define ENABLE_FP16_SUPPORT
    #define USE_16BIT_ADDRESS_SPACE
#endif

#ifdef ENABLE_FP16_SUPPORT
    #extension GL_AMD_gpu_shader_half_float : enable
    #extension GL_AMD_gpu_shader_half_float_fetch : enable
    //#extension GL_KHR_gpu_shader_float16 : enable // unknown status
#endif

#ifdef ENABLE_INT16_SUPPORT
    #extension GL_AMD_gpu_shader_int16 : enable
#endif

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
    #define WORK_SIZE 1024
#else
    #define WORK_SIZE 768
#endif
#endif

#ifndef WORK_SIZE
    #define WORK_SIZE 64
#endif

#define LOCAL_SIZE_LAYOUT layout(local_size_x=WORK_SIZE)in

#define USE_MORTON_32


#endif
