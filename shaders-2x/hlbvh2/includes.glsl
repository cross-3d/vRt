
// Morton codes and geometry counters

layout ( std430, binding = 0, set = 0 )  buffer MortoncodesB {
    uvec2 Mortoncodes[];
};

layout ( std430, binding = 1, set = 0 )  buffer MortoncodesIndicesB {
    int MortoncodesIndices[];
};

layout ( std430, binding = 3, set = 0 )  buffer LeafsB {
    HlbvhNode Leafs[];
};

layout ( std430, binding = 4, set = 0 )  buffer bvhBoxesWorkB { 
    vec4 bvhBoxesWork[][2];
};

layout ( std430, binding = 5, set = 0 )  buffer FlagsB {
    int Flags[];
};

layout ( std430, binding = 6, set = 0 )  buffer ActivesB {
    int Actives[][2];
};

layout ( std430, binding = 7, set = 0 )  buffer LeafIndicesB {
    int LeafIndices[];
};

layout ( std430, binding = 8, set = 0 )  buffer CountersB {
    int aCounter;
    int lCounter;
    int cCounter;
    int nCounter;

    int aCounter2;
    int lCounter2;
    int cCounter2;
    int nCounter2;
};





#ifdef USE_F32_BVH
layout ( std430, binding = 12, set = 0 )  writeonly buffer bvhBoxesResultingB { vec4 bvhBoxesResulting[][4]; };
#else
layout ( std430, binding = 12, set = 0 )  writeonly buffer bvhBoxesResultingB { uvec2 bvhBoxesResulting[][4]; }; 
#endif

//layout ( std430, binding = 11, set = 0 )  buffer bvhMetaB { ivec4 bvhMeta[]; };
layout ( binding = 11, set = 0, rgba32i ) uniform iimageBuffer bvhMeta;

struct BVHCreatorUniformStruct {
    mat4x4 transform;
    mat4x4 transformInv;
    mat4x4 projection;
    mat4x4 projectionInv;
    int leafCount, r0, r1, r2;
};

layout ( std430, binding = 10, set = 0 )  readonly buffer bvhBlockB { BVHCreatorUniformStruct creatorUniform; } bvhBlock;

bbox calcTriBox(in mat3x4 triverts) {
    bbox result;
    result.mn = min3_wrap(triverts[0], triverts[1], triverts[2]);
    result.mx = max3_wrap(triverts[0], triverts[1], triverts[2]);
    result.mn -= 1e-5f;
    result.mx += 1e-5f;
    return result;
}
