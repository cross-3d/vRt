
// default definitions
#include "./bvh-traverse-state.glsl"

// intersection current state
struct PrimitiveState {
    vec4 lastIntersection, orig;
#ifdef USE_FAST_INTERSECTION
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
    ((mem.x&1)==1 && (mem.y-mem.x)==1); // additional checking;
};

void resetEntry(in bool VALID) {
    { traverseState.entryIDBase = (VALID && (currentState == BVH_STATE_TOP || INSTANCE_ID >= 0)) ? BVH_ENTRY : -1; };
    [[flatten]] if (currentState == BVH_STATE_BOTTOM) { stackState = BvhSubState( traverseState.entryIDBase, mint_t(0), mint_t(0) ); };
};

// 
bool validIdxTop    (in    int idx) { return (currentState == BVH_STATE_TOP || (idx >= 0 && idx > topLevelEntry)) && ( topLevelEntry >= 0 ); };
bool validIdx       (inout int idx) { return idx >= 0 && idx >= traverseState.entryIDBase; };
bool validIdxEntry  (inout int idx) { return validIdx(idx) && idx != traverseState.entryIDBase; };
bool validIdxIncluse(inout int idx) { return (currentState != BVH_STATE_TOP || idx != traverseState.entryIDBase) && validIdx(idx); };

// 
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
    resetEntry(intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe));
#else
    resetEntry(true);
#endif

    // traversing inputs
    primitiveState.orig = torig, traverseState.directInv = fvec4_(dirproj), traverseState.minusOrig = fvec4_(torig * traverseState.directInv);
};

// kill switch when traversing 
void triggerSwitch(inout uint stateTo) { currentState = stateTo, initTraversing(true, -1, ORIGINAL_ORIGIN, ORIGINAL_DIRECTION); };
void switchStateTo(in uint stateTo, in int instanceTo, in bool valid) {
    [[flatten]] if (currentState != stateTo) {
        const bool ToTop = (stateTo == BVH_STATE_TOP), ToBottom = !ToTop;
        [[flatten]] if (ToTop) { lstack = traverseCache.stack[cacheID]; } else { traverseCache.stack[cacheID] = lstack; };
        [[flatten]] if (ToTop) { stackState = resrvState; } else { resrvState = stackState; };
        INSTANCE_ID = instanceTo, triggerSwitch(stateTo);
    };
};

// triangle intersection, when it found
void doIntersection( in bool ISEND, in bool PVALID, inout bool switched ) {
    const int elementID = exchange(traverseState.defElementID,0)-1;
    const uint CSTATE = currentState; const bool IsTop = (CSTATE == BVH_STATE_TOP), IsBottom = !IsTop;
    
    [[flatten]] if (PVALID && IsBottom) {
        //vec3 uvt = vec3(0.f.xx, INFINITY); const float nearT = fma(primitiveState.lastIntersection.z,fpOne,fpInner);
        vec3 uvt = vec3(0.f.xx, INFINITY); bool isvalid = true;
        intersectTriangle(primitiveState.orig, primitiveState.dir, elementID, uvt, isvalid);

        //const float tdiff = nearT-d, tmax = SFN;
        //[[flatten]] if (tdiff >= -tmax && d < N_INFINITY && isvalid) {
            //[[flatten]] if (tdiff >= tmax || elementID >= floatBitsToInt(primitiveState.lastIntersection.w)) {
            [[flatten]] if ( isvalid && uvt.z <= primitiveState.lastIntersection.z ) {
                primitiveState.lastIntersection = vec4(uvt, intBitsToFloat(elementID+1)); LAST_INSTANCE = INSTANCE_ID;
            };
        //};
    };
    
    [[flatten]] if (IsTop ? PVALID : ISEND) 
        [[flatten]] if (validIdxTop(IsTop ? stackState.idx : resrvState.idx)) { switchStateTo( uint(IsTop), (IsTop ? elementID : -1), true), switched = true; };
};

// 
#include "./bvh-traverse-process.glsl"
