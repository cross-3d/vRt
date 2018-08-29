
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
#ifdef USE_F32_BVH
     vec4 cbox[3];
    ivec4 meta;
#else
    uvec2 cbox[3];
    ivec4 meta;
#endif
};

layout ( binding = 2, set = 1, std430 ) restrict buffer bvhBoxesResultingB { BTYPE_ bvhNodes[]; };

bbox_t calcTriBox(in mat3x4 triverts) {
    bbox_t result;
    result.mn = min3_wrap(triverts[0], triverts[1], triverts[2]) - 5e-4f;
    result.mx = max3_wrap(triverts[0], triverts[1], triverts[2]) + 5e-4f;
    return result;
};
