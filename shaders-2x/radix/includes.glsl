#include "../include/driver.glsl"
#include "../include/mathlib.glsl"

#define OUR_INVOC_TERM
uint Radice_Idx = 0;
uint Lane_Idx = 0;
uint Local_Idx = 0;
uint Wave_Idx = 0;


#ifdef INTEL_PLATFORM
#undef Wave_Size
#define Wave_Size 32u
#endif

#include "../include/ballotlib.glsl"


// radices 4-bit
#define BITS_PER_PASS 4
#define RADICES 16
#define RADICES_MASK 0xf
//#define AFFINITION 1
#define AFFINITION 16 // hyper-threaded powers

// general work groups
#define BLOCK_SIZE (Wave_Size * RADICES / AFFINITION) // how bigger block size, then more priority going to radices (i.e. BLOCK_SIZE / Wave_Size)
#define BLOCK_SIZE_RT (gl_WorkGroupSize.x)
#define WRK_SIZE_RT ((BLOCK_SIZE_RT/Wave_Size_RT) * gl_NumWorkGroups.y)

#define uvec_wave uint
#define bvec_wave bool
#define uvec64_wave uint64_t
#define bvec2_wave bvec2

#if defined(ENABLE_AMD_INSTRUCTION_SET) && defined(ENABLE_AMD_INT16)
#define uint_rdc_wave uint16_t
#define uint_rdc_wave_lcm uint
#define uint_rdc_wave_2 u16vec2
#else
#define uint_rdc_wave uint
#define uint_rdc_wave_lcm uint
#define uint_rdc_wave_2 uvec2
#endif

// pointer of...
#define WPTR uint
#define WPTR2 uvec2

#define READ_LANE(V, I) (uint(I >= 0 && I < Wave_Size_RT) * readLane(V, I))

uint BFE(in uint ua, in uint o, in uint n) {
    return BFE_HW(ua, int(o), int(n));
}

//planned extended support
//uint64_t BFE(inout uint64_t ua, in uint64_t o, in uint64_t n) {
uint BFE(in uvec2 ua, in uint o, in uint n) {
    return uint(o >= 32u ? BFE_HW(ua.y, int(o-32u), int(n)) : BFE_HW(ua.x, int(o), int(n)));
}



#define KEYTYPE uvec2
//#define KEYTYPE uvec_wave
layout (std430, binding = 20, set = 0 )  readonly buffer KeyInB {KEYTYPE KeyIn[]; };
layout (std430, binding = 21, set = 0 )  readonly buffer ValueInB {uint ValueIn[]; };
layout (std430, binding = 24, set = 0 )  readonly buffer VarsB {
    uint NumKeys;
    uint Shift;
    uint Descending;
    uint IsSigned;
};
layout (std430, binding = 25, set = 0 )  writeonly buffer KeyTmpB {KEYTYPE KeyTmp[]; };
layout (std430, binding = 26, set = 0 )  writeonly buffer ValueTmpB {uint ValueTmp[]; };
layout (std430, binding = 27, set = 0 )  buffer HistogramB {uint Histogram[]; };
layout (std430, binding = 28, set = 0 )  buffer PrefixSumB {uint PrefixSum[]; };


struct blocks_info { uint count; uint offset; uint limit; uint r0; };

blocks_info get_blocks_info(in uint n) {
    uint block_tile = Wave_Size_RT;
    uint block_size = tiled(n, gl_NumWorkGroups.x);
    uint block_count = tiled(n, block_tile * gl_NumWorkGroups.x);
    uint block_offset = gl_WorkGroupID.x * block_tile * block_count;
    return blocks_info(block_count, block_offset, min(block_offset + tiled(block_size, block_tile)*block_tile, n), 0);
}

