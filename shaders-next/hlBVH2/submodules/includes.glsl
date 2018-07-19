
// Morton codes and geometry counters

layout ( binding = 0, set = 0, std430 ) coherent buffer MortoncodesB {
#ifdef USE_MORTON_32
    uint Mortoncodes[];
#else
    uvec2 Mortoncodes[];
#endif
};

layout ( binding = 1, set = 0, std430 ) coherent buffer MortoncodesIndicesB {
    int MortoncodesIndices[];
};

layout ( binding = 3, set = 0, std430 ) coherent buffer LeafsB {
    leaf_t Leafs[];
};

layout ( binding = 4, set = 0, std430 ) restrict buffer bvhBoxesWorkB { 
    vec4 bvhBoxesWork[][2];
};

layout ( binding = 5, set = 0, std430 ) restrict buffer FlagsB {
    int Flags[];
};

layout ( binding = 6, set = 0, std430 ) coherent buffer ActivesB {
    //int Actives[][2];
    int Actives[];
};

layout ( binding = 7, set = 0, std430 ) coherent buffer LeafIndicesB {
    int LeafIndices[];
};

layout ( binding = 8, set = 0, std430 ) restrict buffer CountersB {
    int aCounter[2];
    int vtCounters[6];
};


#ifdef USE_F32_BVH
#define BTYPE_ vec4
#else
#define BTYPE_ uvec2
#endif

layout ( binding = 2, set = 1, std430 ) buffer bvhBoxesResultingB { BTYPE_ bvhBoxesResulting[][4]; };
layout ( binding = 3, set = 1, rgba32i ) uniform iimageBuffer bvhMeta;

bbox_t calcTriBox(in mat3x4 triverts) {
    bbox_t result;
    result.mn = min3_wrap(triverts[0], triverts[1], triverts[2]) - 5e-4f;
    result.mx = max3_wrap(triverts[0], triverts[1], triverts[2]) + 5e-4f;
    return result;
}
