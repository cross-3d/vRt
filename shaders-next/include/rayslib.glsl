#ifndef _RAYS_H
#define _RAYS_H

// include
#include "../include/mathlib.glsl"
#include "../include/morton.glsl"
#include "../include/ballotlib.glsl"

// paging optimized tiling
const int R_BLOCK_WIDTH = 8, R_BLOCK_HEIGHT = 8;
const int R_BLOCK_SIZE = R_BLOCK_WIDTH * R_BLOCK_HEIGHT;

// basic ray tracing buffers
layout ( std430, binding = 0, set = 0 ) buffer VT_RAYS {VtRay rays[];};
layout ( std430, binding = 1, set = 0 ) buffer VT_HITS {HitData hits[];};
layout ( std430, binding = 2, set = 0 ) buffer VT_CLOSEST_HITS {int closestHits[];};
layout ( std430, binding = 3, set = 0 ) buffer VT_MISS_HITS {int missHits[];};
layout ( std430, binding = 4, set = 0 ) buffer VT_HIT_PAYLOAD { HitPayload hitPayload; };

// system canvas info
layout ( std430, binding = 6, set = 0 ) readonly buffer VT_CANVAS_INFO {
    ivec2 size;
} canvasInfo;

// counters
layout ( std430, binding = 7, set = 0 ) buffer arcounterB { 
    int rayCounter;
    int hitCounter;
    int closestHitCounter;
    int missHitCounter;

    int payloadHitCounter;
    int r0, r1, r2;
};

// atomic counters with subgroups
initAtomicSubgroupIncFunction(rayCounter, atomicIncRayCount, 1, int)
initAtomicSubgroupIncFunction(hitCounter, atomicIncHitCount, 1, int)
initAtomicSubgroupIncFunction(closestHitCounter, atomicIncClosestHitCount, 1, int)
initAtomicSubgroupIncFunction(missHitCounter, atomicIncMissHitCount, 1, int)
initAtomicSubgroupIncFunction(payloadHitCounter, atomicIncPayloadHitCount, 1, int)

// alpha version of low level ray emitter
int vtEmitRays(in VtRay ray) {
    int rayID = atomicIncRayCount();
    rays[rayID] = ray;
    return rayID;
}

VtRay vtFetchRay(in int lidx){
    return rays[lidx];
}

int vtVerifyClosestHit(in int closestId){
    int id = atomicIncClosestHitCount();
    closestHits[id] = closestId+1;
    return id;
}

int vtVerifyMissedHit(in int missId){
    int id = atomicIncMissHitCount();
    missHits[id] = missId+1;
    return id;
}


int vtClosestId(in int globalID){
    return closestHits[id]-1;
}

#endif
