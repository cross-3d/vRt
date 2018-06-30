#ifndef _RAYS_H
#define _RAYS_H

// include
#include "../include/mathlib.glsl"
#include "../include/morton.glsl"
#include "../include/ballotlib.glsl"



struct VtHitPayload {
    // hit shaded data
    vec4 normalHeight;
    vec4 albedo;
    vec4 emission;
    vec4 specularGlossiness;
};

// paging optimized tiling
const int R_BLOCK_WIDTH = 8, R_BLOCK_HEIGHT = 8;
const int R_BLOCK_SIZE = R_BLOCK_WIDTH * R_BLOCK_HEIGHT;

// basic ray tracing buffers
layout ( std430, binding = 0, set = 0 ) buffer VT_RAYS {VtRay rays[];};
layout ( std430, binding = 1, set = 0 ) buffer VT_HITS {VtHitData hits[];};
layout ( std430, binding = 2, set = 0 ) buffer VT_CLOSEST_HITS {int closestHits[];};
layout ( std430, binding = 3, set = 0 ) buffer VT_MISS_HITS {int missHits[];};
layout ( std430, binding = 4, set = 0 ) buffer VT_HIT_PAYLOAD { VtHitPayload hitPayload[]; };
layout ( std430, binding = 5, set = 0 ) buffer VT_RAY_INDICES {int rayGroupIndices[];};

// system canvas info
layout ( std430, binding = 6, set = 0 ) readonly buffer VT_CANVAS_INFO {
    ivec2 size; int iteration, closestHitOffset;
    int currentGroup, maxRayCount, r1, r2;
} stageUniform;

// counters
layout ( std430, binding = 7, set = 0 ) buffer VT_RT_COUNTERS { 
    int rayCounter;
    int hitCounter;
    int closestHitCounterCurrent;
    int closestHitCounter;

    int missHitCounter;
    int payloadHitCounter;
    int blockSpaceCounter; // 9th line counter
    int attribCounter;
};

// imported from satellite (blocky indicing)
#ifdef USE_16BIT_ADDRESS_SPACE
layout ( std430, binding = 8, set = 0 ) buffer VT_9_LINE { uint16_t ispace[][R_BLOCK_SIZE]; };
#define m16i(b,i) (int(ispace[b][i])-1)
#define m16s(a,b,i) (ispace[b][i] = uint16_t(a+1))
#else
layout ( std430, binding = 8, set = 0 ) buffer VT_9_LINE { highp uint ispace[][R_BLOCK_SIZE]; };
#define m16i(b,i) (int(ispace[b][i])-1)
#define m16s(a,b,i) (ispace[b][i] = uint(a+1))
#endif

// ray and hit linking buffer
layout ( rgba32ui, binding = 10, set = 0 ) uniform uimageBuffer rayLink;
layout ( rgba32f,  binding = 11, set = 0 ) uniform imageBuffer attributes;


layout ( std430, binding = 12, set = 0 ) buffer VT_GROUPS_COUNTERS {
    int rayTypedCounter[4];
    int closestHitTypedCounter[4];
    int missHitTypedCounter[4];
};

layout ( std430, binding = 13, set = 0 ) readonly buffer VT_RAY_INDICES_READ {int rayGroupIndicesRead[];};
layout ( std430, binding = 14, set = 0 ) readonly buffer VT_GROUPS_COUNTERS_READ {
    int rayTypedCounterRead[4];
    int closestHitTypedCounterRead[4];
    int missHitTypedCounterRead[4];
};



// atomic counters with subgroups
initAtomicSubgroupIncFunction(attribCounter, atomicIncAttribCount, 1, int)
initAtomicSubgroupIncFunction(rayCounter, atomicIncRayCount, 1, int)
initAtomicSubgroupIncFunction(hitCounter, atomicIncHitCount, 1, int)
initAtomicSubgroupIncFunction(closestHitCounterCurrent, atomicIncClosestHitCount, 1, int)
initAtomicSubgroupIncFunction(missHitCounter, atomicIncMissHitCount, 1, int)
initAtomicSubgroupIncFunction(payloadHitCounter, atomicIncPayloadHitCount, 1, int)
initAtomicSubgroupIncFunction(blockSpaceCounter, atomicIncblockSpaceCount, 1, int)
initAtomicSubgroupIncFunctionTarget(rayTypedCounter[WHERE], atomicIncRayTypedCount, 1, int)
initAtomicSubgroupIncFunctionTarget(closestHitTypedCounter[WHERE], atomicIncClosestHitTypedCount, 1, int)
initAtomicSubgroupIncFunctionTarget(missHitTypedCounter[WHERE], atomicIncMissHitTypedCount, 1, int)


int makeAttribID(in int hAttribID, in int sub) {
    return (hAttribID-1)*ATTRIB_EXTENT + sub;
}

//initAtomicSubgroupIncFunction(blockSpaceCounter, atomicIncblockSpaceCount, 1, int)

// alpha version of low level ray emitter
int vtEmitRays(in VtRay ray, in uvec2 c2d, in uint group) {
    const uint type = group;
    int rayID = atomicIncRayCount();
    parameteri(RAY_TYPE, ray.dcolor.y, int(type));
    
    rays[rayID] = ray; 
    imageStore(rayLink, rayID, uvec4(0u, p2x(c2d), 0u.xx));
    //imageStore(rayGroupIndices[type], atomicIncRayTypedCount(type), uint(rayID+1).xxxx);
    int gID = atomicIncRayTypedCount(type);
    if (gID < stageUniform.maxRayCount) rayGroupIndices[gID*4+type] = (rayID+1);
    return rayID;
}


int vtFetchHitIdc(in int lidx) {
    return int(imageLoad(rayLink, lidx).x)-1;
}

uvec2 vtFetchIndex(in int lidx){
    uint c2dp = imageLoad(rayLink, lidx).y;
    return up2x(c2dp);
}


VtRay vtFetchRay(in int lidx) {
    return rays[lidx];
}


int vtVerifyClosestHit(in int closestId, in int g){
    int id = g < 0 ? atomicIncClosestHitCount() : atomicIncClosestHitTypedCount(g);
    closestHits[id*5+(g+1)] = closestId+1;
    return id;
}

int vtVerifyMissedHit(in int missId, in int g){
    //int id = g < 0 ? atomicIncMissHitCount() : atomicIncMissHitTypedCount(g);
    int id = atomicIncMissHitTypedCount(g);
    missHits[id*5+(g+1)] = missId+1;
    return id;
}

int vtClosestId(in int id, in int g) {return closestHits[id*5+(g+1)]-1; }
int vtMissId(in int id, in int g){ return missHits[id*5+(g+1)]-1; }
int vtVerifyClosestHit(in int closestId) { int id = atomicIncClosestHitCount(); closestHits[id*5] = closestId+1; return id; }
int vtVerifyMissedHit(in int missId) { int id = atomicIncMissHitCount(); missHits[id*5] = missId+1; return id; }
int vtClosestId(in int id){ return vtClosestId(id, -1); }
int vtMissId(in int id){ return vtMissId(id, -1); }


#endif
