
// default definitions
#include "./bvh-traverse-state.glsl"

// intersection current state
struct PrimitiveState {
    vec4 lastIntersection, orig;
#ifdef VRT_USE_FAST_INTERSECTION
    vec4 dir;
#else
    int axis; mat3 iM;
#endif
} primitiveState;


// used for re-init traversing 
vec3 ORIGINAL_ORIGIN = vec3(0.f); dirtype_t ORIGINAL_DIRECTION = dirtype_t(0);


// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void resetEntry(in bool valid) { 
    valid == valid && INSTANCE_ID >= 0;

    [[flatten]] if (currentState == BVH_STATE_TOP) {
        traverseState.maxElements = bvhBlockTop.primitiveCount, traverseState.diffOffset = floatBitsToInt(0.f);
        traverseState.entryIDBase = (valid ? bvhBlockTop.entryID : -1);
    } else {
        traverseState.maxElements = bvhBlockIn .primitiveCount, traverseState.diffOffset = floatBitsToInt(0.f);
        traverseState.entryIDBase = (valid ? bvhBlockIn .entryID : -1);
    };

    // bottom states should also reset own stacks, top levels doesn't require it
    traverseState.idx = valid ? traverseState.entryIDBase : -1, traverseState.defElementID = 0;
};

bool validIdx(in int idx){
    return traverseState.entryIDBase >= 0 && idx >= traverseState.entryIDBase && idx >= 0 && idx != -1;
};


// initialize state 
void initTraversing( in bool valid, in int eht, in vec3 orig, in dirtype_t pdir ) {
    //[[flatten]] if (currentState == BVH_STATE_TOP && eht.x >= 0) primitiveState.lastIntersection = hits[eht].uvt;

    // relative origin and vector ( also, preparing mat3x4 support ) 
    // in task-based traversing will have universal transformation for BVH traversing and transforming in dimensions 
    vec4 torig = 0.f.xxxx, torigTo = 0.f.xxxx;
    [[flatten]] if (currentState == BVH_STATE_TOP) {
        torig = -uniteBoxTop(vec4(mult4(bvhBlockTop.transform, vec4(orig, 1.f)).xyz, 1.f)), torigTo = uniteBoxTop(vec4(mult4(bvhBlockTop.transform, vec4(orig, 1.f) + vec4(dcts(pdir).xyz, 0.f)).xyz, 1.f));
    } else {
        torig = -uniteBox   (vec4(mult4(bvhInstance.transform, vec4(orig, 1.f)).xyz, 1.f)), torigTo = uniteBox   (vec4(mult4(bvhInstance.transform, vec4(orig, 1.f) + vec4(dcts(pdir).xyz, 0.f)).xyz, 1.f));
    }; const vec4 tdir = torigTo+torig;

    const vec4 dirct = tdir*invlen, dirproj = 1.f / precIssue(dirct);
    primitiveState.dir = primitiveState.orig = dirct;

    // test intersection with main box
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    const   vec3 interm = fma(0.5f.xxxx, 2.f.xxxx, 1.f.xxxx).xyz;
    const   vec2 bside2 = vec2(-fpOne, fpOne);
    const mat3x2 bndsf2 = mat3x2( bside2*interm.x, bside2*interm.y, bside2*interm.z );
    resetEntry(valid);

    // initial traversing state
    //valid = valid && intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe), resetEntry(valid);

    // traversing inputs
    traverseState.directInv = fvec4_(dirproj), traverseState.minusOrig = fvec4_(vec4(fma(fvec4_(torig), traverseState.directInv, ftype_(intBitsToFloat(traverseState.diffOffset)).xxxx)));
    primitiveState.orig = fma(primitiveState.orig, traverseState.diffOffset.xxxx, torig);
};


// kill switch when traversing 
void switchStateTo(in uint stateTo, in int instanceTo, in bool valid) {
    const uint CSTATE = currentState;
    [[flatten]] if (CSTATE != stateTo) { INSTANCE_ID = instanceTo;
        [[flatten]] if (CSTATE == BVH_STATE_TOP && stateTo == BVH_STATE_BOTTOM) { traverseState.idxTop = traverseState.idx; };
#ifdef ENABLE_STACK_SWITCH
        [[flatten]] if (CSTATE == BVH_STATE_TOP && stateTo == BVH_STATE_BOTTOM) {
            traverseCache.pages[_cacheID] = lstack;
            traverseState.stackPtrTop = traverseState.stackPtr, traverseState.pageIDTop = traverseState.pageID;
        };
#endif

        currentState = stateTo; initTraversing(valid, -1, ORIGINAL_ORIGIN, ORIGINAL_DIRECTION);

        // restoration of stack state
        [[flatten]] if (CSTATE == BVH_STATE_BOTTOM && stateTo == BVH_STATE_TOP) { traverseState.idx = traverseState.idxTop; };
#ifdef ENABLE_STACK_SWITCH
        [[flatten]] if (CSTATE == BVH_STATE_BOTTOM && stateTo == BVH_STATE_TOP) {
            traverseState.stackPtr = traverseState.stackPtrTop, traverseState.pageID = traverseState.pageIDTop;
            lstack = traverseCache.pages[_cacheID];
        };
        [[flatten]] if (CSTATE == BVH_STATE_TOP && stateTo == BVH_STATE_BOTTOM) {
            traverseState.stackPtr = 0, traverseState.pageID = 0;
        };
#endif
    };
};


// triangle intersection, when it found
void doIntersection(in bool isvalid) {
    const int elementID = traverseState.defElementID-1; traverseState.defElementID = 0;
    isvalid = isvalid && elementID >= 0 && elementID  < traverseState.maxElements;

    const uint CSTATE = currentState;
    [[flatten]] if (isvalid && CSTATE == BVH_STATE_TOP) { 
        switchStateTo(BVH_STATE_BOTTOM, elementID, isvalid);
    } //else 

    [[flatten]] if (isvalid && CSTATE == BVH_STATE_BOTTOM) {
        vec2 uv = vec2(0.f.xx); const float nearT = fma(primitiveState.lastIntersection.z,fpOne,fpInner), d = 
#ifdef VRT_USE_FAST_INTERSECTION
            intersectTriangle(primitiveState.orig, primitiveState.dir, elementID, uv.xy, isvalid, nearT);
#else
            intersectTriangle(primitiveState.orig, primitiveState.iM, primitiveState.axis, elementID, uv.xy, isvalid);
#endif

        const float tdiff = nearT-d, tmax = 0.f;
        [[flatten]] if (tdiff >= -tmax && d < N_INFINITY && isvalid) {
            [[flatten]] if (abs(tdiff) > tmax || elementID >= floatBitsToInt(primitiveState.lastIntersection.w)) {
                primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(elementID+1));
            };
        };
    };
};

#include "./bvh-traverse-process.glsl"
