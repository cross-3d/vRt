
// default definitions

#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif


// global memory stack pages (256-bit)
const highp int localStackSize = 8, pageCount = 4, computedStackSize = localStackSize*pageCount, max_iteraction = 8192;
layout ( std430, binding = _CACHE_BINDING, set = 0 ) coherent buffer VT_PAGE_SYSTEM { int pages[][8]; };


// BVH traversing state
struct BvhTraverseState {
    int idx, defTriangleID, stackPtr, cacheID, pageID; float minDist; // vec4, vec2
    bvec3 boxSide; bool fcontinue; // bvec4
    fvec4_ minusOrig, directInv; // vec4 of 32-bits
} traverseState;
#define _continue traverseState.fcontinue


// intersection current state
struct PrimitiveState {
    vec4 lastIntersection, orig;
#ifdef VRT_USE_FAST_INTERSECTION
    vec4 dir;
#else
    int axis; mat3 iM;
#endif
} primitiveState;


// stack system of current BVH traverser
shared int localStack[WORK_SIZE][localStackSize];
#define lstack localStack[Local_Idx]

int loadStack() {
    [[flatten]] if (traverseState.stackPtr <= 0 && traverseState.pageID > 0) { 
        lstack = pages[traverseState.cacheID*pageCount + (--traverseState.pageID)]; traverseState.stackPtr = localStackSize; 
    };
    int idx = --traverseState.stackPtr, rsl = idx >= 0 ? lstack[idx] : -1; traverseState.stackPtr = max(traverseState.stackPtr, 0); return rsl;
};

void storeStack(in int rsl) {
    [[flatten]] if (rsl >= 0) {
        if (traverseState.stackPtr >= localStackSize && traverseState.pageID < pageCount) {
            pages[traverseState.cacheID*pageCount + (traverseState.pageID++)] = lstack; traverseState.stackPtr = 0;
        }
        int idx = traverseState.stackPtr++; [[flatten]] if (idx < localStackSize) lstack[idx] = rsl; traverseState.stackPtr = min(traverseState.stackPtr, localStackSize);
    }
};


// triangle intersection, when it found
void doIntersection() {
    const bool isvalid = true;//traverseState.defTriangleID > 0;
    vec2 uv = vec2(0.f.xx); const float d = 
#ifdef VRT_USE_FAST_INTERSECTION
        intersectTriangle(primitiveState.orig, primitiveState.dir, traverseState.defTriangleID-1, uv.xy, isvalid, primitiveState.lastIntersection.z);
#else
        intersectTriangle(primitiveState.orig, primitiveState.iM, primitiveState.axis, traverseState.defTriangleID-1, uv.xy, isvalid);
#endif
#define nearhit (primitiveState.lastIntersection.z+(0.f))

    [[flatten]] if (d <= nearhit && d >= traverseState.minDist.x && d < (INFINITY-IOFF)) {
        [[flatten]] if (abs(primitiveState.lastIntersection.z-d) > 0.f || traverseState.defTriangleID > floatBitsToInt(primitiveState.lastIntersection.w)) {
            primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defTriangleID));
        }
    } traverseState.defTriangleID=0;
}


// corrections of box intersection
#ifdef USE_F32_BVH 
const float hCorrection = 1.f;
#else
const float16_t hCorrection = 1.hf;
#endif


// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void traverseBvh2(in bool valid, in int eht, in vec3 orig, in vec2 pdir) {

    // relative origin and vector
    const vec4 torig = -divW(mult4( bvhBlock.transform, vec4(orig, 1.0f))), torigTo = divW(mult4( bvhBlock.transform, vec4(orig, 1.0f) + vec4(dcts(pdir.xy), 0.f))), tdir = torigTo+torig;
    
    // different length of box space and global space
    //const float dirlen = length(tdir), invlen = precIssue(dirlen);
    const float dirlen = 1.f, invlen = precIssue(dirlen);
    const vec4 direct = tdir * invlen, dirproj = 1.f / precIssue(direct);

    // limitation of distance
    #define bsgn traverseState.boxSide
    bsgn = bvec3((ivec3(sign(dirproj.xyz))+1)>>1);

    // pre-calculate for triangle intersections
#ifdef VRT_USE_FAST_INTERSECTION
    primitiveState.dir = direct;
#else
    // calculate longest axis
    primitiveState.axis = 2; {
        const vec3 drs = abs(direct);
        if (drs.y >= drs.x && drs.y > drs.z) primitiveState.axis = 1;
        if (drs.x >= drs.z && drs.x > drs.y) primitiveState.axis = 0;
        if (drs.z >= drs.y && drs.z > drs.x) primitiveState.axis = 2;
    }

    // calculate affine matrices
    const vec4 vm = vec4(-direct, 1.f) / (primitiveState.axis == 0 ? direct.x : (primitiveState.axis == 1 ? direct.y : direct.z));
    primitiveState.iM = transpose(mat3(
        primitiveState.axis == 0 ? vm.wyz : vec3(1.f,0.f,0.f),
        primitiveState.axis == 1 ? vm.xwz : vec3(0.f,1.f,0.f),
        primitiveState.axis == 2 ? vm.xyw : vec3(0.f,0.f,1.f)
    ));
#endif

    // test intersection with main box
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    const vec2 bndsf2 = 1.f * vec2(-1.f-1e-4f, 1.f+1e-4f);
    const int entry = (valid ? BVH_ENTRY : -1);

    // initial traversing state
    //traverseState.idx = intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, mat3x2(bndsf2,bndsf2,bndsf2), nfe) ? entry : -1, traverseState.idx = nfe.x >= (INFINITY-IOFF) ? -1 : traverseState.idx;
    traverseState.idx = entry, traverseState.idx = nfe.x >= (INFINITY-IOFF) ? -1 : traverseState.idx; // unable to intersect the root box 
    traverseState.stackPtr = 0, traverseState.pageID = 0, traverseState.defTriangleID = 0, traverseState.minDist = 0.f; const float diffOffset = min(-nfe.x, 0.f);
    traverseState.minusOrig = fvec4_(fma(fvec4_(torig), fvec4_(dirproj), fvec4_(diffOffset.xxxx)));
    traverseState.directInv = fvec4_(dirproj) * hCorrection;
    
    // initial intersection state
    primitiveState.orig = fma(direct, diffOffset.xxxx, torig);
    primitiveState.lastIntersection = eht >= 0 ? hits[eht].uvt : vec4(0.f.xx, INFINITY, FINT_ZERO), primitiveState.lastIntersection.z = min(primitiveState.lastIntersection.z, INFINITY);
    primitiveState.lastIntersection.z = fma(primitiveState.lastIntersection.z, dirlen, diffOffset);

    // two loop based BVH traversing
    [[dependency_infinite]] for (int hi=0;hi<max_iteraction;hi++) {
        [[flatten]] if (traverseState.idx >= 0 && traverseState.defTriangleID <= 0) 
        { [[dependency_infinite]] for (;hi<max_iteraction;hi++) {  _continue = false;
            
            const ivec2 cnode = traverseState.idx >= 0 ? bvhNodes[traverseState.idx].meta.xy : (0).xx;
            [[flatten]] if (isLeaf(cnode)) { traverseState.defTriangleID = cnode.x; } // if leaf, defer for intersection 
            else { // if not leaf, intersect with nodes
                const fmat3x4_ bbox2x = fmat3x4_(bvhNodes[traverseState.idx].cbox[0], bvhNodes[traverseState.idx].cbox[1], bvhNodes[traverseState.idx].cbox[2]);
                lowp bvec2_ childIntersect = bvec2_(traverseState.idx >= 0) & intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, traverseState.boxSide.xyz, bbox2x, nfe);
                childIntersect &= bvec2_(lessThanEqual(nfe.xy, primitiveState.lastIntersection.zz)); // it increase FPS by filtering nodes by first triangle intersection
                
                // 
                int fmask = int((childIntersect.y<<1u)|childIntersect.x);
                [[flatten]] if (fmask > 0) {
                    int primary = -1, secondary = -1;
                    [[flatten]] if (fmask == 3) { fmask &= nfe.x<=nfe.y ? 1 : 2, secondary = cnode.x^(fmask>>1); }; // if both has intersection
                    primary = cnode.x^(fmask&1);
                    
                    // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                    // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                    ivec2 snode = secondary >= 0 ? bvhNodes[secondary].meta.xy : (0).xx;
                    [[flatten]] if (isLeaf(snode)) { traverseState.defTriangleID = snode.x, secondary = -1; }; storeStack(secondary);

                    // set traversing node id
                    traverseState.idx = primary, _continue = true;
                }
            }

            // if all threads had intersection, or does not given any results, break for processing
            [[flatten]] if (!_continue) { traverseState.idx = loadStack(); } // load from stack 
            [[flatten]] IFANY (traverseState.defTriangleID > 0 || traverseState.idx < 0) { break; } // 
        }};
        
        // every-step solving 
        [[flatten]] if (traverseState.defTriangleID > 0) { doIntersection(); } // if has triangle, do intersection
        [[flatten]] if (traverseState.idx < 0) { break; } // if no to traversing - breaking
    };

    // correction of hit distance
    primitiveState.lastIntersection.z = fma(primitiveState.lastIntersection.z, invlen, -diffOffset*invlen);
    primitiveState.lastIntersection.z = min(primitiveState.lastIntersection.z, INFINITY); // clamp by infinite
}
