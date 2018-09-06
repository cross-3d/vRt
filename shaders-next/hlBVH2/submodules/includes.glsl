
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


#ifdef USE_F32_BVH
const float fpIndiv       = 16.f *  0.00000011920928955078125f; // Fraction n/16777216
const float fpCorrect     = 2.f  *  0.00006103515625f;          // Fraction n/16384
const float fpInner       = 2.f  *  0.00006103515625f;          // Fraction n/16384
#else
const float     fpIndiv   = 1024.f * 0.00000011920928955078125f; // Fraction n/16777216
const float16_t fpCorrect = 8.hf   * 0.00006103515625hf;         // Fraction n/16384
const float     fpInner   = 8.f    * 0.00006103515625f;          // Fraction n/16384
#endif


bbox_t calcTriBox(in mat3x4 triverts) {
    bbox_t result;
    result.mn = min3_wrap(triverts[0], triverts[1], triverts[2]) - fpInner;
    result.mx = max3_wrap(triverts[0], triverts[1], triverts[2]) + fpInner;
    return result;
};

bbox_t calcTriBox(in mat3x4 triverts, in vec4 range) {
    bbox_t result;
    result.mn = min3_wrap(triverts[0], triverts[1], triverts[2]) - max(range*fpIndiv,fpInner);
    result.mx = max3_wrap(triverts[0], triverts[1], triverts[2]) + max(range*fpIndiv,fpInner);
    return result;
};
