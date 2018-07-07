
// default definitions

#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif

#ifndef USE_STACKLESS_BVH
//const int localStackSize = 8, extStackSize = 32, computedStackSize = localStackSize+extStackSize;
const int localStackSize = 8, pageCount = 4, computedStackSize = localStackSize*pageCount;

const int max_iteraction = 8192;
//layout ( r32i, binding = _CACHE_BINDING, set = 0 ) uniform iimageBuffer texelPages;
layout ( std430, binding = _CACHE_BINDING, set = 0 ) coherent buffer VT_PAGE_SYSTEM { int pages[][8]; };
#endif

_RAY_TYPE currentRayTmp;

struct BvhTraverseState {
    int idx, defTriangleID, stackPtr, cacheID, pageID;
    float distMult, diffOffset, cutOut;
    fvec4_ minusOrig, directInv; bvec4_ boxSide;

#ifdef USE_STACKLESS_BVH
    uint64_t bitStack, bitStack2;
#endif
} traverseState;

struct PrimitiveState {
    vec4 lastIntersection;
#ifdef USE_FAST_INTERSECTION
    vec4 dir;
#else
    int axis; mat3 iM;
#endif
} primitiveState;


#ifndef USE_STACKLESS_BVH
shared int localStack[WORK_SIZE][localStackSize];
#define lstack localStack[Local_Idx]

int loadStack() {
    if (traverseState.stackPtr <= 0 && traverseState.pageID > 0) { 
        lstack = pages[traverseState.cacheID*pageCount + (--traverseState.pageID)]; traverseState.stackPtr = localStackSize; 
    };
    int idx = --traverseState.stackPtr, rsl = lstack[idx]; return rsl;
}

void storeStack(in int rsl) {
    if (traverseState.stackPtr >= localStackSize && traverseState.pageID < pageCount) {
        pages[traverseState.cacheID*pageCount + (traverseState.pageID++)] = lstack; traverseState.stackPtr = 0;
    }
    int idx = traverseState.stackPtr++; lstack[idx] = rsl;
}

bool stackIsFull() { return traverseState.stackPtr >= localStackSize && traverseState.pageID >= pageCount; }
bool stackIsEmpty() { return traverseState.stackPtr <= 0 && traverseState.pageID <= 0; }
#endif

void doIntersection() {
    vec2 uv = vec2(0.f.xx); const float d = 
#ifdef USE_FAST_INTERSECTION
        intersectTriangle(currentRayTmp.origin.xyz, primitiveState.dir.xyz, traverseState.defTriangleID, uv.xy, traverseState.defTriangleID >= 0);
#else
        intersectTriangle(currentRayTmp.origin.xyz, primitiveState.iM, primitiveState.axis, traverseState.defTriangleID, uv.xy, traverseState.defTriangleID >= 0);
#endif
#define nearhit primitiveState.lastIntersection.z

    //[[flatten]]
    if (d < nearhit) { traverseState.cutOut = d * traverseState.distMult - traverseState.diffOffset; }

    //[[flatten]]
    if (d < INFINITY && d <= nearhit) { primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defTriangleID+1)); } traverseState.defTriangleID=-1;
    //if (d < INFINITY && d <= nearhit) { primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(exchange(traverseState.defTriangleID,-1)+1)); }
}

void traverseBvh2(in bool_ valid, in int eht) {
    vec3 origin = currentRayTmp.origin.xyz;
    vec3 direct = dcts(currentRayTmp.cdirect.xy);

    // test constants
    vec3 
        torig = -divW(mult4( bvhBlock.transform, vec4(origin, 1.0f))).xyz,
        torigTo = divW(mult4( bvhBlock.transform, vec4(origin+direct, 1.0f))).xyz,
        dirproj = torigTo+torig;

    // make vector for box intersection
    float dirlen = length(dirproj); dirproj /= dirlen, dirproj = 1.f.xxx / vec3(precIssue(dirproj.x), precIssue(dirproj.y), precIssue(dirproj.z));

    // limitation of distance
    bvec3_ bsgn = (bvec3_(sign(dirproj)*ftype_(1.0001f))+true_)>>true_;

    // initial state
    traverseState.defTriangleID = -1;
    traverseState.distMult = dirlen;
    traverseState.diffOffset = 0.f;
#ifdef USE_STACKLESS_BVH
    traverseState.bitStack = 0ul;
#endif

    primitiveState.lastIntersection = eht >= 0 ? hits[eht].uvt : vec4(0.f.xx, INFINITY, FINT_ZERO);
    
#ifdef USE_FAST_INTERSECTION
    primitiveState.dir = vec4(direct, 1.f);
#else
    // calculate longest axis
    primitiveState.axis = 2;
    {
        vec3 drs = abs(direct); 
        if (drs.y >= drs.x && drs.y > drs.z) primitiveState.axis = 1;
        if (drs.x >= drs.z && drs.x > drs.y) primitiveState.axis = 0;
        if (drs.z >= drs.y && drs.z > drs.x) primitiveState.axis = 2;
    }

    // calculate affine matrices
    vec4 vm = vec4(-direct, 1.f) / (primitiveState.axis == 0 ? direct.x : (primitiveState.axis == 1 ? direct.y : direct.z));
    primitiveState.iM = mat3(
        primitiveState.axis == 0 ? vm.wyz : vec3(1.f,0.f,0.f),
        primitiveState.axis == 1 ? vm.xwz : vec3(0.f,1.f,0.f),
        primitiveState.axis == 2 ? vm.xyw : vec3(0.f,0.f,1.f)
    );
#endif

    // test intersection with main box
    float near = -INFINITY, far = INFINITY;
    const vec2 bndsf2 = vec2(-1.0005f, 1.0005f);
    traverseState.idx = SSC(intersectCubeF32Single(torig*dirproj, dirproj, bsgn, mat3x2(bndsf2, bndsf2, bndsf2), near, far)) ? (SSC(valid) ? BVH_ENTRY : -1) : -1;
    traverseState.stackPtr = 0, traverseState.pageID = 0;
    traverseState.diffOffset = max(near, 0.f);
    traverseState.directInv.xyz = fvec3_(dirproj);
    traverseState.minusOrig.xyz = fma(fvec3_(torig), fvec3_(dirproj), -fvec3_(traverseState.diffOffset).xxx);
    traverseState.boxSide.xyz = bsgn;
    traverseState.cutOut = primitiveState.lastIntersection.z * traverseState.distMult - traverseState.diffOffset; 
    
    // begin of traverse BVH 
    ivec4 cnode = traverseState.idx >= 0 ? (texelFetch(bvhMeta, traverseState.idx)-1) : (-1).xxxx;
    for (int hi=0;hi<max_iteraction;hi++) {
        SB_BARRIER
        IFALL (traverseState.idx < 0) break; // if traverse can't live
        if (traverseState.idx >= 0) { for (;hi<max_iteraction;hi++) {
            bool _continue = false;

            // if not leaf and not wrong
            if (cnode.x != cnode.y) {
                vec2 nears = INFINITY.xx, fars = INFINITY.xx;
                bvec2_ childIntersect = bvec2_((traverseState.idx >= 0).xx);

                // intersect boxes
                const int _cmp = cnode.x >> 1;
                childIntersect &= intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, traverseState.boxSide.xyz, 
#ifdef USE_F32_BVH
                    fmat3x4_(bvhBoxes[_cmp][0], bvhBoxes[_cmp][1], bvhBoxes[_cmp][2]),
                    fmat3x4_(vec4(0.f), vec4(0.f), vec4(0.f))
#else
                    fmat3x4_(UNPACK_HF(bvhBoxes[_cmp][0].xy), UNPACK_HF(bvhBoxes[_cmp][1].xy), UNPACK_HF(bvhBoxes[_cmp][2].xy)),
                    fmat3x4_(vec4(0.f), vec4(0.f), vec4(0.f))
#endif
                , nears, fars);
                childIntersect &= bvec2_(lessThanEqual(nears, traverseState.cutOut.xx));

                int fmask = (childIntersect.x + childIntersect.y*2)-1; // mask of intersection

                [[flatten]]
                if (fmask >= 0) {
                    _continue = true;

#ifdef USE_STACKLESS_BVH
                    traverseState.bitStack <<= 1;
#endif

                    [[flatten]]
                    if (fmask == 2) { // if both has intersection
                        ivec2 ordered = cnode.xx + (SSC(lessEqualF(nears.x, nears.y)) ? ivec2(0,1) : ivec2(1,0));
                        traverseState.idx = ordered.x;
#ifdef USE_STACKLESS_BVH
                        IF (all(childIntersect)) traverseState.bitStack |= 1ul; 
#else
                        IF (all(childIntersect) & bool_(!stackIsFull())) storeStack(ordered.y);
#endif
                    } else {
                        traverseState.idx = cnode.x + fmask;
                    }

                    cnode = traverseState.idx >= 0 ? (texelFetch(bvhMeta, traverseState.idx)-1) : (-1).xxxx;
                }
            }
            
            // if leaf, defer for intersection 
            if (cnode.x == cnode.y && traverseState.defTriangleID < 0) {
                traverseState.defTriangleID = cnode.x;
            }

            if (!_continue) {
#ifdef USE_STACKLESS_BVH
                // stackless
                for (int bi=0;bi<64;bi++) {
                    if ((traverseState.bitStack&1ul)!=0ul || traverseState.bitStack==0ul) break;
                    traverseState.bitStack >>= 1;
                    traverseState.idx = traverseState.idx >= 0 ? (texelFetch(bvhMeta, traverseState.idx).z-1) : -1;
                }

                // goto to sibling or break travers
                if (traverseState.bitStack!=0ul && traverseState.idx >= 0) {
                    traverseState.idx += traverseState.idx%2==0?1:-1; traverseState.bitStack &= ~1ul;
                } else {
                    traverseState.idx = -1;
                }
#else
                // stacked 
                if (!stackIsEmpty()) {
                    traverseState.idx = loadStack();
                } else {
                    traverseState.idx = -1;
                }
#endif
                cnode = traverseState.idx >= 0 ? (texelFetch(bvhMeta, traverseState.idx)-1) : (-1).xxxx;
            } _continue = false;

            IFANY (traverseState.defTriangleID >= 0 || traverseState.idx < 0) { SB_BARRIER break; }
        }}

        SB_BARRIER

        IFANY (traverseState.defTriangleID >= 0 || traverseState.idx < 0) { SB_BARRIER doIntersection(); }
    }
}

