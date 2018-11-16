
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
//vec3 ORIGINAL_ORIGIN = vec3(0.f); dirtype_t ORIGINAL_DIRECTION = dirtype_t(0);
#define ORIGINAL_ORIGIN rays[RPG_OFF+RAY_ID].origin.xyz
#define ORIGINAL_DIRECTION rays[RPG_OFF+RAY_ID].cdirect

// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x >= 1 && mem.x==mem.y && (currentState == BVH_STATE_TOP || mem.x <= traverseState.maxElements); };
bool isnLeaf(in ivec2 mem) { return mem.x >= 1 && //mem.x!=mem.y && 
    (mem.x&1)==1 && (mem.y-mem.x)==1; // additional checking;
};


void resetEntry(in bool VALID) {
    VALID = VALID && INSTANCE_ID >= 0;//, MAX_ELEMENTS = VALID ? (currentState == BVH_STATE_TOP ? bvhBlockTop.primitiveCount : bvhBlockIn.primitiveCount) : 0, VALID = VALID && MAX_ELEMENTS > 0;
    traverseState.diffOffset = floatBitsToInt(0.f);
    traverseState.entryIDBase = VALID ? (currentState == BVH_STATE_TOP ? bvhBlockTop.entryID : bvhBlockIn.entryID) : -1;
    [[flatten]] if (currentState == BVH_STATE_BOTTOM || !VALID) {
        traverseState.idx = traverseState.entryIDBase, traverseState.stackPtr = 0, traverseState.pageID = 0, lstack[sidx] = -1;
    };
};


bool validIdxTop(in int idx) {
    return idx >= 0 && idx > bvhBlockTop.entryID;
};

bool validIdx(in int idx) {
    return idx >= 0 && idx >= traverseState.entryIDBase;
};

bool validIdxIncluse(in int idx) {
    return validIdx(idx) && idx != bvhBlockTop.entryID;
};


vec4 uniteBoxLv(in vec4 pt) {
#ifdef EXPERIMENTAL_UNORM16_BVH
    return (currentState == BVH_STATE_TOP ? uniteBoxTop(pt) : uniteBox(pt));
#else
    return pt;
#endif
};


// initialize state 
// TODO: reincarnate watertight triangle intersection method
void initTraversing( in bool valid, in int eht, in vec3 orig, in dirtype_t pdir ) {
    // relative origin and vector ( also, preparing mat3x4 support ) 
    // in task-based traversing will have universal transformation for BVH traversing and transforming in dimensions 
    const mat3x4 TRANSFORM_IN = currentState == BVH_STATE_TOP ? mat3x4(bvhBlockTop.transform) : mat3x4(instanceTransform);
    const vec4 
        torig   = -uniteBoxLv(vec4(mult4(TRANSFORM_IN, vec4(orig, 1.f)).xyz, 1.f)), 
        torigTo =  uniteBoxLv(vec4(mult4(TRANSFORM_IN, vec4(orig, 1.f) + vec4(dcts(pdir).xyz, 0.f)).xyz, 1.f)), 
        tdir    =  torigTo+torig;

    const vec4 dirct = tdir*invlen, dirproj = 1.f / precIssue(dirct);
    primitiveState.dir = primitiveState.orig = dirct;

    // test intersection with main box
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    const   vec3 interm = fma(0.5f.xxxx, 2.f.xxxx, fpInner.xxxx).xyz;
    const   vec2 bside2 = vec2(-fpOne, fpOne);
    const mat3x2 bndsf2 = mat3x2( bside2*interm.x, bside2*interm.y, bside2*interm.z );

    // initial traversing state
#ifdef EXPERIMENTAL_UNORM16_BVH
    [[flatten]] if (valid) {
        valid = intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe);
    };
#endif
    resetEntry(valid);

    // traversing inputs
    traverseState.directInv = fvec4_(dirproj), traverseState.minusOrig = fvec4_(vec4(fma(fvec4_(torig), traverseState.directInv, ftype_(intBitsToFloat(traverseState.diffOffset)).xxxx)));
    primitiveState.orig = fma(primitiveState.orig, traverseState.diffOffset.xxxx, torig);
};

// 
void triggerSwitch(in uint stateTo){
    currentState = stateTo; initTraversing(true, -1, ORIGINAL_ORIGIN, ORIGINAL_DIRECTION);
};

// kill switch when traversing 
void switchStateTo(in uint stateTo, in int instanceTo, in bool valid) {
    [[flatten]] if (currentState != stateTo) { 
        [[flatten]] if (stateTo == BVH_STATE_BOTTOM && !traverseState.saved) {
            traverseState.idxTop = traverseState.idx, traverseState.stackPtrTop = traverseState.stackPtr, traverseState.pageIDTop = traverseState.pageID, traverseState.saved = true;
            traverseCache.stack[_cacheID] = lstack;
        };

        [[flatten]] if (stateTo == BVH_STATE_TOP && traverseState.saved) {
            traverseState.idx = traverseState.idxTop, traverseState.stackPtr = traverseState.stackPtrTop, traverseState.pageID = traverseState.pageIDTop, traverseState.saved = false;
            traverseState.idxTop = -1, traverseState.stackPtrTop = -1, traverseState.pageIDTop = -1;
            lstack = traverseCache.stack[_cacheID];
        };

        INSTANCE_ID = instanceTo;
        if (valid) triggerSwitch(stateTo);
    };
};

// triangle intersection, when it found
void doIntersection( in bool isvalid ) {
    const int elementID = exchange(traverseState.defElementID,0)-1;
    isvalid = isvalid && elementID >= 0; //&& elementID < traverseState.maxElements;

    const uint CSTATE = currentState;
    [[flatten]] if (isvalid && CSTATE == BVH_STATE_TOP) { 
        switchStateTo(BVH_STATE_BOTTOM, elementID, isvalid);
    };

    [[flatten]] if (isvalid && CSTATE == BVH_STATE_BOTTOM) {
        //vec2 uv = vec2(0.f.xx); const float nearT = fma(primitiveState.lastIntersection.z,fpOne,fpInner), d = 
        vec2 uv = vec2(0.f.xx); const float d = intersectTriangle(primitiveState.orig, primitiveState.dir, elementID, uv.xy, isvalid);

        //const float tdiff = nearT-d, tmax = SFN;
        //[[flatten]] if (tdiff >= -tmax && d < N_INFINITY && isvalid) {
            //[[flatten]] if (tdiff >= tmax || elementID >= floatBitsToInt(primitiveState.lastIntersection.w)) {
            [[flatten]] if ( isvalid && d.x <= primitiveState.lastIntersection.z ) {
                primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(elementID+1)); LAST_INSTANCE = INSTANCE_ID;
            };
        //};
    };
};

// 
#include "./bvh-traverse-process.glsl"
