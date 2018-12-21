#define EXTEND_LOCAL_GROUPS
#include "../include/driver.glsl"
#include "../include/mathlib.glsl"
#include "../include/ballotlib.glsl"


// roundly like http://www.heterogeneouscompute.org/wordpress/wp-content/uploads/2011/06/RadixSort.pdf
// partially of https://vgc.poly.edu/~csilva/papers/cgf.pdf + and wide subgroup adaptation
// practice maximal throughput: 256x16 (of Nx256)


// 2-bit
//#define BITS_PER_PASS 2
//#define RADICES 4u
//#define RADICES_MASK 0x3u

#ifdef ENABLE_TURING_INSTRUCTION_SET
// 8-bit (risen again, but Turing only)
#define BITS_PER_PASS 8
#define RADICES 256u
#define RADICES_MASK 0xFFu
#define READ_U8
#else
// 4-bit
#define BITS_PER_PASS 4
#define RADICES 16u
#define RADICES_MASK 0xFu
#define SIMPLER_SORT
#endif


// general work groups
#define Wave_Size_RX Wave_Size_RT
#define Wave_Count_RX Wave_Count_RT //(gl_WorkGroupSize.x / Wave_Size_RT.x)
//#define BLOCK_SIZE (Wave_Size * RADICES / AFFINITION) // how bigger block size, then more priority going to radices (i.e. BLOCK_SIZE / Wave_Size)


//#if defined(ENABLE_TURING_INSTRUCTION_SET)
    #define VEC_SIZE 4u
    #define VEC_MULT VEC_SIZE
    #define VEC_SHIF 2u
    #define VEC_SEQU WPTRX(Wave_Idx) // yes, yes!
    #define KTYPE utype_v//utype_t[VEC_SIZE]

    #define WPTRX uint
    #define BOOLX bool

// 
#ifdef ENABLE_TURING_INSTRUCTION_SET
#define Wave_Count VEC_SIZE
#else
#define Wave_Count 16u
#endif

// default values
#ifndef BLOCK_SIZE
#define BLOCK_SIZE (Wave_Size*Wave_Count)
#endif


#define BLOCK_SIZE_RT (gl_WorkGroupSize.x)
#define WRK_SIZE_RT (gl_NumWorkGroups.y * Wave_Count_RX)


#define PREFER_UNPACKED
#define utype_t u8x1_t
#define utype_v u8x4_t

#ifdef USE_MORTON_32
#define KEYTYPE uint32_t
lowp uint BFE(in uint32_t ua, in int o, in int n) { return BFE_HW(ua, o, n); }
#else
#define KEYTYPE u32vec2
lowp uint BFE(in u32vec2 ua, in int o, in int n) { return uint(o >= 32 ? BFE_HW(ua.y, o-32, n) : BFE_HW(ua.x, o, n)); }
#endif

struct RadicePropStruct { uint Descending, IsSigned; };

#ifdef COPY_HACK_IDENTIFY
#define INDIR 0
#define OUTDIR 1
#else
#define INDIR 1
#define OUTDIR 0
#endif

// used when filling
const KEYTYPE OutOfRange = KEYTYPE(0xFFFFFFFFu);

//#define KEYTYPE uint
//#ifdef READ_U8
layout ( binding = 0, set = INDIR, align_ssbo )  readonly subgroupcoherent buffer KeyInU8B {uint8_t[4] Key8n[]; };
//#endif
layout ( binding = 0, set = INDIR, align_ssbo )  readonly subgroupcoherent buffer KeyInB {KEYTYPE KeyIn[]; };
layout ( binding = 1, set = INDIR, align_ssbo )  readonly subgroupcoherent buffer ValueInB {uint ValueIn[]; };
layout ( binding = 0, set = OUTDIR, align_ssbo )  subgroupcoherent buffer KeyTmpB {KEYTYPE KeyTmp[]; };
layout ( binding = 1, set = OUTDIR, align_ssbo )  subgroupcoherent buffer ValueTmpB {uint ValueTmp[]; };
layout ( binding = 2, set = 0, align_ssbo )  readonly buffer VarsB { RadicePropStruct radProps[]; };
layout ( binding = 3, set = 0, align_ssbo )  subgroupcoherent buffer HistogramB {uint Histogram[]; };
layout ( binding = 4, set = 0, align_ssbo )  subgroupcoherent buffer PrefixSumB {uint PrefixSum[]; };

// push constant in radix sort
layout ( push_constant ) uniform PushBlock { uint NumKeys; int Shift; } push_block;

// division of radix sort
struct blocks_info { uint count, offset, limit; };
blocks_info get_blocks_info(in uint n) {
    // tiling should be strictly same
    const uint block_tile = Wave_Size_RT << 2u;//VEC_SHIF;
    const uint block_size = tiled(n, gl_NumWorkGroups.x);
    const uint block_count = tiled(n, block_tile * gl_NumWorkGroups.x);
    const uint block_offset = gl_WorkGroupID.x * block_tile * block_count;
    return blocks_info(block_count, block_offset, min(block_offset + tiled(block_size, block_tile)*block_tile, n));
};

#ifdef PREFER_UNPACKED
#define upfunc(x) (x)
#else
#define upfunc(x) up4x_8(x)
#endif
