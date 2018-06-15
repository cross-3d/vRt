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
layout ( std430, binding = 4, set = 0 ) buffer VT_HIT_PAYLOAD { HitPayload hitPayload[]; };
layout ( std430, binding = 5, set = 0 ) buffer VT_RAY_INDICES {int rayIndices[];};

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
    int blockSpaceCounter; // 9th line counter
    int r0, r1;
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


// atomic counters with subgroups
initAtomicSubgroupIncFunction(rayCounter, atomicIncRayCount, 1, int)
initAtomicSubgroupIncFunction(hitCounter, atomicIncHitCount, 1, int)
initAtomicSubgroupIncFunction(closestHitCounter, atomicIncClosestHitCount, 1, int)
initAtomicSubgroupIncFunction(missHitCounter, atomicIncMissHitCount, 1, int)
initAtomicSubgroupIncFunction(payloadHitCounter, atomicIncPayloadHitCount, 1, int)
initAtomicSubgroupIncFunction(blockSpaceCounter, atomicIncblockSpaceCount, 1, int)

// alpha version of low level ray emitter
int vtEmitRays(in VtRay ray) {
    int rayID = atomicIncRayCount();
    rays[rayID] = ray; imageStore(rayLink, rayID, uvec4(0xFFFFFFFFu));
    return rayID;
}


int vtFetchHitIdc(in int lidx) {
    return int(imageLoad(rayLink, lidx).x)-1;
}

VtRay vtFetchRay(in int lidx) {
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


int vtClosestId(in int id){
    return closestHits[id]-1;
}

int vtMissId(in int id){
    return missHits[id]-1;
}


#endif
