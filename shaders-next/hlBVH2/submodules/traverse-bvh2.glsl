
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

//_RAY_TYPE currentRayTmp;

struct BvhTraverseState {
    int idx, defTriangleID, stackPtr, cacheID, pageID; lowp bvec4_ boxSide;
    //float distMult, diffOffset, cutOut;
    fvec4_ minusOrig, directInv;

#ifdef USE_STACKLESS_BVH
    uint64_t bitStack, bitStack2;
#endif
} traverseState;

struct PrimitiveState {
    vec4 lastIntersection;
    vec3 orig;
#ifdef USE_FAST_INTERSECTION
    vec3 dir;
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
    const bool isvalid = true; //traverseState.defTriangleID >= 0;
    vec2 uv = vec2(0.f.xx); const float d = 
#ifdef USE_FAST_INTERSECTION
        intersectTriangle(primitiveState.orig.xyz, primitiveState.dir.xyz, traverseState.defTriangleID, uv.xy, isvalid);
#else
        intersectTriangle(primitiveState.orig.xyz, primitiveState.iM, primitiveState.axis, traverseState.defTriangleID, uv.xy, isvalid);
#endif
#define nearhit primitiveState.lastIntersection.z

    //[[flatten]]
    //if (d < nearhit) { traverseState.cutOut = fma(d, traverseState.distMult, -traverseState.diffOffset); }

    [[flatten]]
    if (d < INFINITY && d <= nearhit) { primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defTriangleID+1)); } traverseState.defTriangleID=-1;
}

void traverseBvh2(in bool valid, in int eht, in vec3 orig, in vec2 pdir) {
    //const vec3 origin = currentRayTmp.origin.xyz, direct = dcts(currentRayTmp.cdirect.xy);

    // test constants
    //vec3 torig = -orig.xyz, dirproj = dcts(pdir.xy);
    vec3 torig = -divW(mult4( bvhBlock.transform, vec4(orig.xyz, 1.0f))).xyz,
         torigTo = divW(mult4( bvhBlock.transform, vec4(orig.xyz+dcts(pdir.xy), 1.0f))).xyz,
         dirproj = torigTo+torig;

    // make vector for box intersection
    const float dirlen = length(dirproj);
    //const float dirlen = 1.f;
    dirproj /= dirlen; const vec3 direct = dirproj; dirproj = 1.f.xxx / vec3(precIssue(dirproj.x), precIssue(dirproj.y), precIssue(dirproj.z));

    // limitation of distance
    lowp bvec3_ bsgn = (bvec3_(sign(dirproj)*ftype_(1.0001f))+true_)>>true_;

    // initial state
    traverseState.defTriangleID = -1;
    //traverseState.distMult = dirlen;
#ifdef USE_STACKLESS_BVH
    traverseState.bitStack = 0ul;
#endif

#ifdef USE_FAST_INTERSECTION
    primitiveState.dir = direct;//vec4(direct, 1.f);
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
    primitiveState.iM = transpose(mat3(
        primitiveState.axis == 0 ? vm.wyz : vec3(1.f,0.f,0.f),
        primitiveState.axis == 1 ? vm.xwz : vec3(0.f,1.f,0.f),
        primitiveState.axis == 2 ? vm.xyw : vec3(0.f,0.f,1.f)
    ));
#endif

    // test intersection with main box
    vec2 nears = (-INFINITY).xx, fars = INFINITY.xx;
    const vec2 bndsf2 = vec2(-1.0005f, 1.0005f);
    const int entry = (valid ? BVH_ENTRY : -1), _cmp = entry >> 1;
    //traverseState.idx = SSC(intersectCubeF32Single(torig*dirproj, dirproj, bsgn, mat3x4(bvhBoxes[_cmp][0], bvhBoxes[_cmp][1], bvhBoxes[_cmp][2]), nears.x, fars.x)) ? entry : -1;
    traverseState.idx = SSC(intersectCubeF32Single(torig*dirproj, dirproj, bsgn, mat3x2(bndsf2,bndsf2,bndsf2), nears.x, fars.x)) ? entry : -1; 
    
    float diffOffset = -max(nears.x, 0.f); 
    primitiveState.orig = fma(direct, diffOffset.xxx, torig);//vec4(fma(direct, diffOffset.xxx, torig), 1.f);
    primitiveState.lastIntersection = eht >= 0 ? hits[eht].uvt : vec4(0.f.xx, INFINITY, FINT_ZERO), primitiveState.lastIntersection.z = fma(primitiveState.lastIntersection.z, dirlen, diffOffset);

    //traverseState.diffOffset = diffOffset;
    traverseState.stackPtr = 0, traverseState.pageID = 0;
    traverseState.directInv.xyz = fvec3_(dirproj);
    traverseState.minusOrig.xyz = fma(fvec3_(torig), fvec3_(dirproj), fvec3_(diffOffset).xxx);
    traverseState.boxSide.xyz = bsgn;

    // begin of traverse BVH 
    ivec4 cnode = traverseState.idx >= 0 ? (texelFetch(bvhMeta, traverseState.idx)-1) : (-1).xxxx;

    [[dependency_infinite]]
    for (int hi=0;hi<max_iteraction;hi++) {
        [[flatten]]
        if (traverseState.idx >= 0 && traverseState.defTriangleID < 0) 
        { [[dependency_infinite]] for (;hi<max_iteraction;hi++) {
            bool _continue = false;

            [[flatten]]
            if (cnode.x == cnode.y) { // if leaf, defer for intersection 
                [[flatten]]
                if (traverseState.defTriangleID < 0) { 
                    traverseState.defTriangleID = cnode.x;
                } else {
                    _continue = true;
                    //continue; 
                }
            } else { // if not leaf, intersect with nodes
                const int _cmp = cnode.x >> 1; // intersect boxes
                lowp bvec2_ childIntersect = bvec2_(traverseState.idx >= 0) & intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, traverseState.boxSide.xyz, 
                    fmat3x4_(bvhBoxes[_cmp][0], bvhBoxes[_cmp][1], bvhBoxes[_cmp][2])
                , nears, fars);

                // it increase FPS by filtering nodes by first triangle intersection
                childIntersect &= bvec2_(lessThanEqual(nears, primitiveState.lastIntersection.zz));
                int fmask = int(childIntersect.x + childIntersect.y*2u)-1; // mask of intersection

                [[flatten]]
                if (fmask >= 0) {
#ifdef USE_STACKLESS_BVH
                    traverseState.bitStack <<= 1;
#endif

                    [[flatten]]
                    if (fmask == 2) { // if both has intersection
                        //ivec2 ordered = cnode.xx + (nears.x<=nears.y ? ivec2(0,1) : ivec2(1,0));
                        ivec2 ordered = nears.x<=nears.y ? cnode.xy : cnode.yx;
                        traverseState.idx = ordered.x;
#ifdef USE_STACKLESS_BVH
                        IF (all(childIntersect)) traverseState.bitStack |= 1ul; 
#else
                        IF (all(childIntersect) & bool_(!stackIsFull())) storeStack(ordered.y);
#endif
                    } else {
                        //traverseState.idx = cnode.x + fmask;
                        traverseState.idx = fmask == 0 ? cnode.x : cnode.y;
                    }

                    cnode = traverseState.idx >= 0 ? (texelFetch(bvhMeta, traverseState.idx)-1) : (-1).xxxx;
                    _continue = true; 
                    //continue;
                }
            }

            [[flatten]]
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
            }

            // if all threads had intersection, or does not given any results, break for processing
            //IFALL
            IFANY 
            (traverseState.defTriangleID >= 0 || traverseState.idx < 0) { break; }
        }}
        
        [[flatten]]
        if (traverseState.defTriangleID >= 0) { doIntersection(); }
        [[flatten]]
        if (traverseState.idx < 0) { break; }
    }

    // correction of hit distance
    primitiveState.lastIntersection.z = fma(primitiveState.lastIntersection.z-diffOffset, 1.f/dirlen, 1e-5f);
}

