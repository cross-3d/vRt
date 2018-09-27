
// Morton codes and geometry counters

layout ( binding = 0, set = 0, std430 ) restrict buffer MortoncodesB {
#ifdef USE_MORTON_32
    uint Mortoncodes[];
#else
    uvec2 Mortoncodes[];
#endif
};

layout ( binding = 1, set = 0, std430 ) restrict buffer MortoncodesIndicesB {
    int MortoncodesIndices[];
};

layout ( binding = 3, set = 0, std430 ) restrict buffer LeafsB {
    leaf_t Leafs[];
};

layout ( binding = 4, set = 0, std430 ) restrict buffer bvhBoxesWorkB { 
    vec4 bvhBoxesWork[][2];
};

layout ( binding = 5, set = 0, std430 ) restrict buffer FlagsB {
    int Flags[];
};

layout ( binding = 6, set = 0, std430 ) restrict buffer ActivesB {
    //int Actives[][2];
    int Actives[];
};

layout ( binding = 7, set = 0, std430 ) restrict buffer LeafIndicesB {
    int LeafIndices[];
};

layout ( binding = 8, set = 0, std430 ) restrict buffer CountersB {
    int aCounter[2];
    int vtCounters[6];
};

struct BTYPE_ {
#if (defined(USE_F32_BVH) || defined(AMD_F16_BVH))
    fvec4_ cbox[3];
#else
    uvec2 cbox[3];
#endif
    ivec4 meta;
};

layout ( binding = 2, set = 1, std430 ) restrict buffer bvhBoxesResultingB { BTYPE_ bvhNodes[]; };

// precision control of boxes
#ifndef fpInner
//#define fpInner 128.f*SFN // as operation 
//#define fpInner 0.0000152587890625
const float fpInner = 0.0000152587890625f;
#endif

#define fpCorrect InZero

bbox_t calcTriBox(in mat3x4 triverts) {
    bbox_t result;
    result.mn = -fpInner + min3_wrap(triverts[0], triverts[1], triverts[2]);
    result.mx =  fpInner + max3_wrap(triverts[0], triverts[1], triverts[2]);
    return result;
};

bbox_t calcTriBox(in mat3x4 triverts, in vec4 vRange) {
    bbox_t result;
    result.mn = fma(vRange, -fpInner.xxxx, min3_wrap(triverts[0], triverts[1], triverts[2]));
    result.mx = fma(vRange, +fpInner.xxxx, max3_wrap(triverts[0], triverts[1], triverts[2]));
    return result;
};
