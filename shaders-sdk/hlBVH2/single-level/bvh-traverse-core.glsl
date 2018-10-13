
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

// triangle intersection, when it found
void doIntersection(in bool isvalid, in float dlen) {
    isvalid = isvalid && traverseState.defElementID > 0 && traverseState.defElementID <= traverseState.maxElements;
    IFANY (isvalid) {
        vec2 uv = vec2(0.f.xx); const float nearT = fma(primitiveState.lastIntersection.z,fpOne,fpInner), d = 
#ifdef VRT_USE_FAST_INTERSECTION
            intersectTriangle(primitiveState.orig, primitiveState.dir, traverseState.defElementID-1, uv.xy, isvalid, nearT);
#else
            intersectTriangle(primitiveState.orig, primitiveState.iM, primitiveState.axis, traverseState.defElementID-1, uv.xy, isvalid);
#endif

        const float tdiff = nearT-d, tmax = 0.f;
        [[flatten]] if (tdiff >= -tmax && d < N_INFINITY && isvalid) {
            [[flatten]] if (abs(tdiff) > tmax || traverseState.defElementID > floatBitsToInt(primitiveState.lastIntersection.w)) {
                primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defElementID));
            };
        };
    }; traverseState.defElementID=0;
};

// 
#ifdef EXPERIMENTAL_INSTANCING_SUPPORT
#define TROOT bvhInstance
#else
#define TROOT bvhBlockTop
#endif

// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void resetEntry(in bool valid) { traverseState.idx = (valid ? BVH_ENTRY : -1), traverseState.stackPtr = 0, traverseState.pageID = 0, traverseState.defElementID = 0; };
void initTraversing( in bool valid, in int eht, in vec3 orig, in dirtype_t pdir ) {
    [[flatten]] if (eht.x >= 0) primitiveState.lastIntersection = hits[eht].uvt;

    // relative origin and vector ( also, preparing mat3x4 support ) 
    // in task-based traversing will have universal transformation for BVH traversing and transforming in dimensions 
    const vec4 torig = -uniteBox(vec4(mult4(TROOT.transform, vec4(orig, 1.f)).xyz, 1.f)), torigTo = uniteBox(vec4(mult4(TROOT.transform, vec4(orig, 1.f) + vec4(dcts(pdir).xyz, 0.f)).xyz, 1.f)), tdir = torigTo+torig;
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

#include "./bvh-traverse-process.glsl"
