#ifndef _RAYS_H
#define _RAYS_H

// include
#include "../include/mathlib.glsl"
#include "../include/morton.glsl"
#include "../include/ballotlib.glsl"

#ifndef RS_SET
#define RS_SET 0
#endif


struct VtHitPayload {
    // hit shaded data
    vec4 normalHeight;
    vec4 albedo;
    vec4 emission;
    vec4 specularGlossiness;
};

// basic ray tracing buffers
layout ( binding = 0, set = RS_SET, align_ssbo ) coherent buffer VT_RAYS { VtRay rays[]; };
layout ( binding = 1, set = RS_SET, align_ssbo ) coherent buffer VT_HITS { VtHitData hits[]; };
layout ( binding = 2, set = RS_SET, align_ssbo ) coherent buffer VT_CLOSEST_HITS { int closestHits[]; };
layout ( binding = 3, set = RS_SET, align_ssbo ) coherent buffer VT_MISS_HITS { int missHits[]; };
layout ( binding = 4, set = RS_SET, align_ssbo ) coherent buffer VT_HIT_PAYLOAD { VtHitPayload hitPayload[]; };
layout ( binding = 5, set = RS_SET, align_ssbo ) coherent buffer VT_RAY_INDICES { int rayGroupIndices[]; };

// system canvas info
layout ( binding = 6, set = RS_SET, align_ssbo ) readonly restrict buffer VT_CANVAS_INFO {
    int currentGroup, maxRayCount, maxHitCount, closestHitOffset;
    ivec2 size; int lastIteration, iteration;
} stageUniform;

#define MAX_HITS stageUniform.maxHitCount
#define MAX_RAYS stageUniform.maxRayCount

#define RPG_OFF (MAX_RAYS*WID)
#define HPG_OFF (MAX_HITS*WID)

#define cntr_t int[2] //ivec2 

// counters
layout ( binding = 7, set = RS_SET, align_ssbo ) restrict buffer VT_RT_COUNTERS { cntr_t vtCounters[8]; };
layout ( binding = 8, set = RS_SET, rg32ui ) uniform uimageBuffer taskList;

// ray and hit linking buffer
layout ( binding = 10, set = RS_SET, r32ui ) uniform uimageBuffer rayLink;
layout ( binding = 11, set = RS_SET, rgba32f ) uniform imageBuffer attributes;
layout ( binding = 12, set = RS_SET, align_ssbo ) restrict buffer VT_GROUPS_COUNTERS {
    cntr_t rayTypedCounter[4];
    cntr_t closestHitTypedCounter[4];
    cntr_t missHitTypedCounter[4];
    cntr_t reservedCounter[4];
};

layout ( binding = 13, set = RS_SET, align_ssbo ) readonly coherent buffer VT_RAY_INDICES_READ {int rayGroupIndicesRead[];};
layout ( binding = 14, set = RS_SET, align_ssbo ) readonly restrict buffer VT_GROUPS_COUNTERS_READ {
    cntr_t rayTypedCounterRead[4];
    cntr_t closestHitTypedCounterRead[4];
    cntr_t missHitTypedCounterRead[4];
    cntr_t reservedCounterRead[4];
};

initAtomicSubgroupIncFunctionTargetBinarity(vtCounters[WHERE], atomicIncVtCounters, 1, int)
initAtomicSubgroupIncFunctionTargetBinarity(rayTypedCounter[WHERE], atomicIncRayTypedCount, 1, int)
initAtomicSubgroupIncFunctionTargetBinarity(closestHitTypedCounter[WHERE], atomicIncClosestHitTypedCount, 1, int)
initAtomicSubgroupIncFunctionTargetBinarity(missHitTypedCounter[WHERE], atomicIncMissHitTypedCount, 1, int)

//initAtomicSubgroupIncFunctionTarget(vtCounters[WHERE], atomicIncVtCounters, 1, int)
//initAtomicSubgroupIncFunctionTarget(rayTypedCounter[WHERE], atomicIncRayTypedCount, 1, int)
//initAtomicSubgroupIncFunctionTarget(closestHitTypedCounter[WHERE], atomicIncClosestHitTypedCount, 1, int)
//initAtomicSubgroupIncFunctionTarget(missHitTypedCounter[WHERE], atomicIncMissHitTypedCount, 1, int)


// aliased values
const uint rGroupDefault = 0u;
#define rayCounter rayTypedCounter[rGroupDefault] //vtCounters[0]
#define hitCounter vtCounters[1]
#define closestHitCounterCurrent vtCounters[2]
#define closestHitCounter vtCounters[3]
#define missHitCounter vtCounters[4]
#define payloadHitCounter vtCounters[5]
#define taskCounter vtCounters[6]
#define attribCounter vtCounters[7]

// aliased functions
int atomicIncRayCount() {return atomicIncVtCounters(0);}
int atomicIncHitCount() {return atomicIncVtCounters(1);}
int atomicIncClosestHitCount() {return atomicIncVtCounters(2);}
int atomicIncMissHitCount() {return atomicIncVtCounters(4);}
int atomicIncPayloadHitCount() {return atomicIncVtCounters(5);}
int atomicIncTaskCount() {return atomicIncVtCounters(6);}
int atomicIncAttribCount() {return atomicIncVtCounters(7);}



#define rHIT hits[HPG_OFF+hitID]
int vtReuseRays(in VtRay ray, in highp uvec2 c2d, in uint type, in lowp int rayID) {
    [[flatten]] if (max3_vec(f16_f32(ray.dcolor)) > 1e-3f) {
        parameteri(RAY_TYPE, ray.dcolor.y, int(type));
        const int rID = atomicIncRayTypedCount(rGroupDefault);//atomicIncRayCount();
        rayID = rayID < 0 ? rID : rayID; rays[RPG_OFF+rayID] = ray;
        [[flatten]] if (rID < MAX_RAYS) {
            const int gID = rID;//atomicIncRayTypedCount(type);
            [[flatten]] if (gID < MAX_RAYS) {
                rayGroupIndices[(2*MAX_RAYS)*(type+1) + RPG_OFF + gID] = (rayGroupIndices[RPG_OFF+rID] = (rayID+1));
            };
        };
    };
    [[flatten]] if (rayID >= 0 && rayID < MAX_RAYS) {
        imageStore(rayLink, int((RPG_OFF+rayID)<<2)|0, 0u.xxxx);
        imageStore(rayLink, int((RPG_OFF+rayID)<<2)|1, p2x_16(c2d).xxxx);
        imageStore(rayLink, int((RPG_OFF+rayID)<<2)|2, 0xFFFFFFFFu.xxxx);
        imageStore(rayLink, int((RPG_OFF+rayID)<<2)|3, 0u.xxxx);
    };
    return rayID;
};

int vtEmitRays(in VtRay ray, in highp uvec2 c2d, in uint type) { return vtReuseRays(ray, c2d, type, -1); };
//int vtFetchHitIdc(in int lidx) { return int(imageAtomicMax(rayLink, (RPG_OFF+lidx)<<2, 0u).x)-1; }; // will be replace in traversing by tasks 
int vtFetchHitIdc(in int lidx) { return int(imageLoad(rayLink, int(RPG_OFF+lidx)<<2).x)-1; };
int vtFetchHitClosest(in int lidx) { return vtFetchHitIdc(lidx); };
//int vtFetchHitClosest(in int lidx) { return int(imageAtomicMax(rayLink, (RPG_OFF+lidx)<<2)|2, 0u).x)-1; };

//
uint vtFetchCode(in int lidx) { return imageLoad(rayLink, int((RPG_OFF+lidx)<<2)|1).x; };
highp uvec2 vtFetchIndex(in int lidx) { return up2x_16(vtFetchCode(lidx)); };
//int vtRayIdx(in int lidx) { return (lidx >= 0 && lidx < MAX_RAYS ? rayGroupIndices[RPG_OFF + lidx] : 0)-1; };
int vtRayIdx(in int lidx) { return (lidx >= 0 && lidx < MAX_RAYS ? rayGroupIndicesRead[RPG_OFF + lidx] : 0)-1; };

int vtVerifyClosestHit(in int closestId, in lowp int g) {
    const int id = g < 0 ? atomicIncClosestHitCount() : atomicIncClosestHitTypedCount(g);
    [[flatten]] if (id >= 0 && id < MAX_HITS) closestHits[(g+1)*(2*MAX_HITS) + HPG_OFF + id] = closestId+1; return id;
};

int vtVerifyMissedHit(in int missId, in lowp int g) {
    const int id = atomicIncMissHitTypedCount(g);
    [[flatten]] if (id >= 0 && id < MAX_HITS) missHits[HPG_OFF + id] = missId+1; return id;
};

int vtClosestId(in int id, in lowp int g) {return (id >= 0 && id < MAX_HITS ? closestHits[(g+1)*(2*MAX_HITS) + HPG_OFF + id] : 0)-1; };
int vtMissId(in int id, in lowp int g) { return (id >= 0 && id < MAX_HITS ? missHits[HPG_OFF + id] : 0)-1; };

int makeAttribID(in int hAttribID, in int sub) { return int(HPG_OFF+(hAttribID-1))*ATTRIB_EXTENT + sub; };

#endif
