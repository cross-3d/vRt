
// Morton codes and geometry counters

#ifdef USE_MORTON_32
    #define morton_t uint
#else
    #define morton_t uvec2
#endif


layout ( binding = 0, set = 0, align_ssbo )   buffer MortoncodesB {
    morton_t Mortoncodes[];
};

layout ( binding = 1, set = 0, align_ssbo )   buffer MortoncodesIndicesB {
    int MortoncodesIndices[];
};

layout ( binding = 3, set = 0, align_ssbo )   buffer LeafsB {
    leaf_t Leafs[];
};

layout ( binding = 4, set = 0, align_ssbo )   buffer bvhBoxesWorkB { 
    vec4 bvhBoxesWork[][2];
};

layout ( binding = 5, set = 0, align_ssbo ) subgroupcoherent buffer FlagsB {
    int Flags[];
};

layout ( binding = 6, set = 0, align_ssbo ) subgroupcoherent buffer ActivesB {
    //int Actives[][2];
    int Actives[];
};

layout ( binding = 7, set = 0, align_ssbo ) subgroupcoherent buffer LeafIndicesB {
    int LeafIndices[];
};

layout ( binding = 8, set = 0, align_ssbo ) subgroupcoherent buffer CountersB {
    int aCounter[2];
    int vtCounters[6];
};


// 
struct VtBuildConst { int primitiveCount, primitiveOffset; };
layout ( binding = 2, set = 0, align_ssbo ) readonly devicecoherent buffer BuildConstB { VtBuildConst buildConst_[]; };
#define buildBlock buildConst_[0]




// precision control of boxes
#ifndef fpInner
#define fpInner InZero
#endif

#ifdef EXPERIMENTAL_UNORM16_BVH
#define fpCorrect 0.0000152587890625f
#else
#define fpCorrect 0.f
#endif

//const float fpCorrect = 0.0000152587890625f;

bbox_t calcTriBox(in mat3x4 triverts) {
    bbox_t result;
    result.mn = -float(fpInner) + min3_wrap(triverts[0], triverts[1], triverts[2]);
    result.mx =  float(fpInner) + max3_wrap(triverts[0], triverts[1], triverts[2]);
    return result;
};

bbox_t calcTriBox(in mat3x4 triverts, in vec4 vRange) {
    bbox_t result;
    result.mn = fma(vRange, -float(fpInner).xxxx, min3_wrap(triverts[0], triverts[1], triverts[2]));
    result.mx = fma(vRange, +float(fpInner).xxxx, max3_wrap(triverts[0], triverts[1], triverts[2]));
    return result;
};
