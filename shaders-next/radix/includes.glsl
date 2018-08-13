#include "../include/driver.glsl"
#include "../include/mathlib.glsl"

//#define OUR_INVOC_TERM
//uint Radice_Idx = 0;
//uint Lane_Idx = 0;
//uint Local_Idx = 0;
//uint Wave_Idx = 0;

#ifdef INTEL_PLATFORM
#define Wave_Size 32u
#endif

#include "../include/ballotlib.glsl"


// MLC optimized
//#define BITS_PER_PASS 2
//#define RADICES 4
//#define RADICES_MASK 0x3

// QLC optimized
//#define BITS_PER_PASS 4
//#define RADICES 16
//#define RADICES_MASK 0xF

// 8-bit
#define BITS_PER_PASS 8
#define RADICES 256
#define RADICES_MASK 0xFF

//#define AFFINITION 1
#ifdef AMD_PLATFORM
#define AFFINITION 16
#else
#define AFFINITION 8
#endif

// general work groups
#define Wave_Count_RX Wave_Count_RT //(gl_WorkGroupSize.x / Wave_Size_RT.x)
//#define BLOCK_SIZE (Wave_Size * RADICES / AFFINITION) // how bigger block size, then more priority going to radices (i.e. BLOCK_SIZE / Wave_Size)
#define BLOCK_SIZE 1024 // prefer many as possible, not tested in NVidia
#define BLOCK_SIZE_RT (gl_WorkGroupSize.x)
#define WRK_SIZE_RT (gl_NumWorkGroups.y * Wave_Count_RX)

#define uvec_wave uint
#define bvec_wave bool
#define uvec64_wave uint64_t
#define bvec2_wave bvec2

//#if defined(ENABLE_AMD_INSTRUCTION_SET) && defined(ENABLE_AMD_INT16)
//#define uint_rdc_wave_lcm uint16_t
//#else
#define uint_rdc_wave_lcm uint
//#endif

// pointer of...
#define WPTR uint
#define WPTR2 uvec2

#define READ_LANE(V, I) (uint(I >= 0 && I < Wave_Size_RT) * readLane(V, I))

#ifdef USE_MORTON_32
#define KEYTYPE uint
uint BFE(in uint ua, in int o, in int n) { return BFE_HW(ua, o, n); }
#else
#define KEYTYPE uvec2
uint BFE(in uvec2 ua, in int o, in int n) { return uint(o >= 32u ? BFE_HW(ua.y, o-32, n) : BFE_HW(ua.x, o, n)); }
#endif

struct RadicePropStruct { uint Descending; uint IsSigned; };

#ifdef COPY_HACK_IDENTIFY
#define INDIR 0
#define OUTDIR 1
#else
#define INDIR 1
#define OUTDIR 0
#endif

//#define KEYTYPE uvec_wave
layout (std430, binding = 0, set = INDIR )  readonly coherent buffer KeyInB {KEYTYPE KeyIn[]; };
layout (std430, binding = 1, set = INDIR )  readonly coherent buffer ValueInB {uint ValueIn[]; };

layout (std430, binding = 0, set = OUTDIR )  coherent buffer KeyTmpB {KEYTYPE KeyTmp[]; };
layout (std430, binding = 1, set = OUTDIR )  coherent buffer ValueTmpB {uint ValueTmp[]; };
layout (std430, binding = 2, set = 0 )  readonly buffer VarsB { RadicePropStruct radProps[]; };
layout (std430, binding = 3, set = 0 )  restrict buffer HistogramB {uint Histogram[]; };
layout (std430, binding = 4, set = 0 )  restrict buffer PrefixSumB {uint PrefixSum[]; };

// push constant in radix sort
layout (push_constant) uniform PushBlock { uint NumKeys; int Shift; } push_block;

// division of radix sort
struct blocks_info { uint count; uint offset; uint limit; uint r0; };
blocks_info get_blocks_info(in uint n) {
    uint block_tile = Wave_Size_RT;
    uint block_size = tiled(n, gl_NumWorkGroups.x);
    uint block_count = tiled(n, block_tile * gl_NumWorkGroups.x);
    uint block_offset = gl_WorkGroupID.x * block_tile * block_count;
    return blocks_info(block_count, block_offset, min(block_offset + tiled(block_size, block_tile)*block_tile, n), 0);
}
