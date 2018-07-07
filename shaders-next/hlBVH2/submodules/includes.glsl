
// Morton codes and geometry counters

layout ( binding = 0, set = 0, std430 )  buffer MortoncodesB {
#ifdef PREFER_64BIT_MORTON_TYPE
    uint64_t Mortoncodes[];
#else
    uvec2 Mortoncodes[];
#endif
};

layout ( binding = 1, set = 0, std430 )  buffer MortoncodesIndicesB {
    int MortoncodesIndices[];
};

layout ( binding = 3, set = 0, std430 )  buffer LeafsB {
    leaf_t Leafs[];
};

layout ( binding = 4, set = 0, std430 )  buffer bvhBoxesWorkB { 
    vec4 bvhBoxesWork[][2];
};

layout ( binding = 5, set = 0, std430 )  buffer FlagsB {
    int Flags[];
};

layout ( binding = 6, set = 0, std430 )  buffer ActivesB {
    int Actives[][2];
};

layout ( binding = 7, set = 0, std430 )  buffer LeafIndicesB {
    int LeafIndices[];
};

layout ( binding = 8, set = 0, std430 )  buffer CountersB {
    int _removed;
    int lCounter;
    int cCounter;
    int nCounter;
    int aCounter[2];

    //int aCounter2;
    //int lCounter2;
    //int cCounter2;
    //int nCounter2;
};




#ifdef USE_F32_BVH
layout ( binding = 2, set = 1, std430 ) buffer bvhBoxesResultingB { vec4 bvhBoxesResulting[][4]; };
#else
layout ( binding = 2, set = 1, std430 ) buffer bvhBoxesResultingB { uvec2 bvhBoxesResulting[][4]; }; 
#endif

layout ( binding = 3, set = 1, rgba32i ) uniform iimageBuffer bvhMeta;

bbox_t calcTriBox(in mat3x4 triverts) {
    bbox_t result;
    result.mn = min3_wrap(triverts[0], triverts[1], triverts[2]) - 5e-4f;
    result.mx = max3_wrap(triverts[0], triverts[1], triverts[2]) + 5e-4f;
    return result;
}
