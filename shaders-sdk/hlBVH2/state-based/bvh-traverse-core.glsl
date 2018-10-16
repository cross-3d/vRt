
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
    [[flatten]] if (currentState == BVH_STATE_TOP) {
        traverseState.entryIDBase = (valid ? bvhBlockTop.entryID : -1);
    } else {
        traverseState.maxElements = bvhBlockIn.primitiveCount, traverseState.diffOffset  = 0.f;
        traverseState.entryIDBase = (valid ? bvhBlockIn.entryID : -1);
    };

    // bottom states should also reset own stacks, top levels doesn't require it
    traverseState.idx = 0, traverseState.stackPtr = 0, traverseState.pageID = 0, traverseState.defElementID = 0;
};


// initialize state 
void initTraversing( in bool valid, in int eht, in vec3 orig, in dirtype_t pdir ) {
    [[flatten]] if (currentState == BVH_STATE_TOP && eht.x >= 0) primitiveState.lastIntersection = hits[eht].uvt;

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
    const   vec3 interm = fma(fpInner.xxxx, 2.f.xxxx, 1.f.xxxx).xyz;
    const   vec2 bside2 = vec2(-fpOne, fpOne);
    const mat3x2 bndsf2 = mat3x2( bside2*interm.x, bside2*interm.y, bside2*interm.z );
    resetEntry(valid);

    // initial traversing state
    //valid = valid && intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe), resetEntry(valid); traverseState.diffOffset = min(-nfe.x, 0.f);

    // traversing inputs
    traverseState.directInv = fvec4_(dirproj), traverseState.minusOrig = fvec4_(vec4(fma(fvec4_(torig), traverseState.directInv, ftype_(traverseState.diffOffset).xxxx)));
    primitiveState.orig = fma(primitiveState.orig, traverseState.diffOffset.xxxx, torig);
};


// kill switch when traversing 
void switchStateTo(in uint stateTo, in int instanceTo){
    [[flatten]] if (currentState != stateTo) {
        // restore hit state 
        //primitiveState.lastIntersection.z = min(fma(primitiveState.lastIntersection.z, invlen, -traverseState.diffOffset*invlen), INFINITY);

        // switch stack in local memory 
        currentState = stateTo;
        //switchStateToStack(stateTo); INSTANCE_ID = instanceTo;

        // every bottom level states requires to partial resetting states 
        //if (stateTo == BVH_STATE_BOTTOM) initTraversing(true, -1, ORIGINAL_ORIGIN, ORIGINAL_DIRECTION);

        // require conversion to correct formation 
        //primitiveState.lastIntersection.z = fma(min(primitiveState.lastIntersection.z, INFINITY), dirlen, traverseState.diffOffset);
    };
};


// triangle intersection, when it found
bool doIntersection(in bool isvalid, in float dlen) {
    bool stateSwitched = false;
    const int elementID = traverseState.defElementID-1; traverseState.defElementID = 0;
    isvalid = isvalid && elementID >= 0 && elementID  < traverseState.maxElements;

    //[[flatten]] if (isvalid && currentState == BVH_STATE_TOP) { 
    //    if (currentState != BVH_STATE_BOTTOM) { switchStateTo(BVH_STATE_BOTTOM, elementID); stateSwitched = true; };
    //};

    [[flatten]] if (isvalid && currentState == BVH_STATE_BOTTOM && !stateSwitched) {
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

    //traverseState.defElementID = 0;
    return stateSwitched;
};

#include "./bvh-traverse-process.glsl"
