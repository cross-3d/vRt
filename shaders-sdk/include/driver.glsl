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


// for NVIDIA doesn't require nonuniform wrapper
#ifdef NVIDIA_PLATFORM
#define DISABLE_NON_UNIFORM_WRAP
#endif

// current features
#define ENABLE_SCALAR_BLOCK_LAYOUT
#define ENABLE_MULTI_BVH

#define EXPERIMENTAL_UNORM16_DIRECTION // packing direction as unorm16x2 (may be inaccurate)
//#define EXPERIMENTAL_UNORM16_BVH     // packing BVH boxes as unorm16x2 (may drop performance)
#define USE_FAST_INTERSECTION          // disable water-tight intersection method (make faster)
#define USE_MOLLER_TRUMBORE            // use Moller Trumbore intersection method (doesn't produce UV holes)
#define USE_INT16_FOR_MORTON           // WARNING! Fixed only 32-bit types support!
//#define USE_INT16_BOOL_PAIR


// AMuDe extensions
#ifdef ENABLE_VEGA_INSTRUCTION_SET
#extension GL_AMD_shader_trinary_minmax : enable
#extension GL_AMD_texture_gather_bias_lod : enable
#extension GL_AMD_shader_image_load_store_lod : enable
#extension GL_AMD_gcn_shader : enable
#endif

// 
//#extension GL_ARB_gpu_shader_int64 : enable // historical
#extension GL_EXT_shader_atomic_int64 : enable
#extension GL_EXT_shader_8bit_storage : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_control_flow_attributes : enable
#extension GL_EXT_shader_image_load_formatted : enable
#extension GL_KHR_memory_scope_semantics : enable // no actual support

// 
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable

// subgroup operations
#extension GL_KHR_shader_subgroup_basic            : require
#extension GL_KHR_shader_subgroup_vote             : require
#extension GL_KHR_shader_subgroup_ballot           : require
#extension GL_KHR_shader_subgroup_arithmetic       : require
#extension GL_KHR_shader_subgroup_shuffle          : enable
#extension GL_KHR_shader_subgroup_shuffle_relative : enable
#ifndef NVIDIA_PLATFORM
#extension GL_KHR_shader_subgroup_clustered        : enable
#else
#extension GL_KHR_shader_subgroup_clustered        : require
#endif

// 
#extension GL_EXT_samplerless_texture_functions : enable
//#extension GL_EXT_subgroupuniform_qualifier : enable


// aligment mode 
#ifdef ENABLE_SCALAR_BLOCK_LAYOUT
#extension GL_EXT_scalar_block_layout : require
#define align_ssbo scalar
#else
#define align_ssbo std430
#endif

// non uniform wrapper
#extension GL_EXT_nonuniform_qualifier : enable
#ifndef DISABLE_NON_UNIFORM_WRAP
#define NonUniform nonuniformEXT
#else
#define NonUniform  
#endif

// if Vega 10 specific
#define ENABLE_NON_UNIFORM_SAMPLER
#ifdef ENABLE_VEGA_INSTRUCTION_SET
    #define ENABLE_INT16_SUPPORT
    #define ENABLE_FP16_SUPPORT
    //#define ENABLE_NATIVE_U8
#endif

// if Turing specific
#ifdef ENABLE_TURING_INSTRUCTION_SET
    #define ENABLE_INT16_SUPPORT
    #define ENABLE_FP16_SUPPORT
    #define ENABLE_NATIVE_U8
    
    #extension GL_NV_shader_subgroup_partitioned : require // volta and above should support it
    #extension GL_NV_compute_shader_derivatives : enable
    //#extension GL_NV_shader_atomic_int64 : enable // unknown status
#endif

// if int16 no supported, use plain int32
#ifndef ENABLE_INT16_SUPPORT
    #undef USE_INT16_FOR_MORTON
    #undef USE_INT16_BOOL_PAIR // use RPM based booleans
#endif

// only under AMD shaders
#ifdef AMD_PLATFORM
    #extension GL_AMD_gpu_shader_half_float_fetch : enable
    #extension GL_AMD_gpu_shader_half_float : enable
    #extension GL_AMD_gpu_shader_int16 : enable
#endif

// platform-oriented compute
#ifndef WORK_SIZE
#ifdef EXTEND_LOCAL_GROUPS
#ifdef ENABLE_VEGA_INSTRUCTION_SET
    #define WORK_SIZE 1024u
#else
    #ifdef ENABLE_TURING_INSTRUCTION_SET
        //#define WORK_SIZE 512u
        //#define WORK_SIZE 768u
        //#define WORK_SIZE 1536u
          #define WORK_SIZE 1024u
    #else
        #ifdef AMD_PLATFORM
            #define WORK_SIZE 512u // best cover for Polaris
        #else
            #define WORK_SIZE 640u // blocks of Pascal 
        #endif
    #endif
#endif
#endif
#endif

#ifndef WORK_SIZE
    #ifdef AMD_PLATFORM
        #define WORK_SIZE 64u
    #else
        #define WORK_SIZE 32u
    #endif
#endif

#define LOCAL_SIZE_LAYOUT layout(local_size_x=WORK_SIZE)in

#define USE_MORTON_32

#ifndef WORK_SIZE_BND
#define WORK_SIZE_BND WORK_SIZE
#endif

#endif
