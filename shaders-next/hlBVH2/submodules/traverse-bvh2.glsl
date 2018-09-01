
// default definitions

#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif


const highp int localStackSize = 8, pageCount = 4, computedStackSize = localStackSize*pageCount, max_iteraction = 8192;
layout ( std430, binding = _CACHE_BINDING, set = 0 ) coherent buffer VT_PAGE_SYSTEM { int pages[][8]; };


struct BvhTraverseState {
    int idx, defTriangleID, stackPtr, cacheID, pageID; bvec3 boxSide; float minDist;
    fvec4_ minusOrig, directInv;
} traverseState;

struct PrimitiveState {
    vec4 lastIntersection, orig;
#ifdef VRT_USE_FAST_INTERSECTION
    vec4 dir;
#else
    int axis; mat3 iM;
#endif
} primitiveState;


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

void doIntersection() {
    const bool isvalid = true;//traverseState.defTriangleID > 0;
    vec2 uv = vec2(0.f.xx); const float d = 
#ifdef VRT_USE_FAST_INTERSECTION
        intersectTriangle(primitiveState.orig, primitiveState.dir, traverseState.defTriangleID-1, uv.xy, isvalid);
#else
        intersectTriangle(primitiveState.orig, primitiveState.iM, primitiveState.axis, traverseState.defTriangleID-1, uv.xy, isvalid);
#endif
#define nearhit (primitiveState.lastIntersection.z+(1e-4f))

    [[flatten]] if (d < INFINITY && d <= nearhit && d >= traverseState.minDist.x) {
        [[flatten]] if (abs(primitiveState.lastIntersection.z-d) > 1e-4f || traverseState.defTriangleID > floatBitsToInt(primitiveState.lastIntersection.w)) {
            primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defTriangleID));
        }
    } traverseState.defTriangleID=0;
}


bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void traverseBvh2(in bool valid, in int eht, in vec3 orig, in vec2 pdir) {

    // test constants
    const vec4 
        torig = -divW(mult4( bvhBlock.transform, vec4(orig, 1.0f))),
        torigTo = divW(mult4( bvhBlock.transform, vec4(orig, 1.0f) + vec4(dcts(pdir.xy), 0.f))),
        tdir = torigTo+torig;

    // make vector for box and triangle intersection
    const float dirlen = length(tdir);
    const vec4 direct = tdir / dirlen;
    const vec4 dirproj = 1.f / (max(abs(direct), 1e-4f)*sign(direct));

    // limitation of distance
    //const bvec3 bsgn = bvec3((ivec3(sign(dirproj.xyz))+1)>>1);
    #define bsgn traverseState.boxSide
    bsgn = bvec3((ivec3(sign(dirproj.xyz))+1)>>1);

    // initial state
    traverseState.defTriangleID = 0;

#ifdef VRT_USE_FAST_INTERSECTION
    primitiveState.dir = direct;
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
    vec4 nfe = vec4(INFINITY.xxxx);
    const vec2 bndsf2 = vec2(-1.0005f, 1.0005f);
    const int entry = (valid ? BVH_ENTRY : -1), _cmp = entry >> 1;
    traverseState.idx = intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, mat3x2(bndsf2,bndsf2,bndsf2), nfe) ? entry : -1;
    traverseState.stackPtr = 0, traverseState.pageID = 0;
    
    const float diffOffset = min(-nfe.x, 0.f);
    primitiveState.orig = fma(direct, diffOffset.xxxx, torig);
    primitiveState.lastIntersection = eht >= 0 ? hits[eht].uvt : vec4(0.f.xx, INFINITY, FINT_ZERO), primitiveState.lastIntersection.z = fma(primitiveState.lastIntersection.z, dirlen, diffOffset);

    // setup min dist for limiting traverse
    traverseState.minDist = fma(traverseState.minDist, dirlen, diffOffset-(1e-4f));
    
#ifdef USE_F32_BVH
    traverseState.directInv = fvec4_(dirproj);
#else
    traverseState.directInv = fvec4_(dirproj)*One1024.xxxx;
#endif
    traverseState.minusOrig = fma(fvec4_(torig), fvec4_(dirproj), fvec4_(diffOffset.xxxx));
    //traverseState.boxSide = bsgn;


    [[dependency_infinite]]
    for (int hi=0;hi<max_iteraction;hi++) {
        [[flatten]]
        if (traverseState.idx >= 0 && traverseState.defTriangleID <= 0) 
        { [[dependency_infinite]] for (;hi<max_iteraction;hi++) {
            bool _continue = false;
            //const ivec2 cnode = (traverseState.idx >= 0 ? bvhNodes[traverseState.idx].meta.xy : (0).xx)-(1).xx;
            const ivec2 cnode = traverseState.idx >= 0 ? bvhNodes[traverseState.idx].meta.xy : (0).xx;

            [[flatten]]
            if (isLeaf(cnode)) { // if leaf, defer for intersection 
                //[[flatten]]
                //if (traverseState.defTriangleID <= 0) { 
                //    traverseState.defTriangleID = cnode.x;
                //} else {
                //    _continue = true;
                    //continue; 
                //}
                traverseState.defTriangleID = cnode.x; // faster traversing mode 
            } else { // if not leaf, intersect with nodes
                lowp bvec2_ childIntersect = bvec2_(traverseState.idx >= 0) & intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, traverseState.boxSide.xyz, 
                    fmat3x4_(bvhNodes[traverseState.idx].cbox[0], bvhNodes[traverseState.idx].cbox[1], bvhNodes[traverseState.idx].cbox[2]), nfe);

                // it increase FPS by filtering nodes by first triangle intersection
                childIntersect &= bvec2_(lessThanEqual(nfe.xy, primitiveState.lastIntersection.zz));
                childIntersect &= bvec2_(greaterThanEqual(nfe.zw, traverseState.minDist.xx));
                
                int fmask = int((childIntersect.y<<1u)|childIntersect.x);
                [[flatten]] if (fmask > 0) {
                    int primary = -1, secondary = -1;
                    [[flatten]] if (fmask == 3) { // if both has intersection
                        fmask &= nfe.x<=nfe.y ? 1 : 2, secondary = cnode.x^(fmask>>1);
                    }; primary = cnode.x^(fmask&1);
                    
                    // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                    // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                    ivec2 snode = secondary >= 0 ? bvhNodes[secondary].meta.xy : (0).xx;
                    [[flatten]] if (isLeaf(snode)) { traverseState.defTriangleID = snode.x, secondary = -1; }; storeStack(secondary);

                    traverseState.idx = primary;
                    _continue = true;
                }
            }

            // if all threads had intersection, or does not given any results, break for processing
            [[flatten]] if (!_continue) { traverseState.idx = loadStack(); } // load from stack 
            [[flatten]] IFANY (traverseState.defTriangleID > 0 || traverseState.idx < 0) { break; }
        }}
        
        [[flatten]] if (traverseState.defTriangleID > 0) { doIntersection(); }
        [[flatten]] if (traverseState.idx < 0) { break; }
    }

    // correction of hit distance
    //primitiveState.lastIntersection.z = max(fma(primitiveState.lastIntersection.z-diffOffset, 1.f/(dirlen*(fma(1.f, 2.f/(1e3f), 1.f))), 1e-4f), 1e-4f);
    primitiveState.lastIntersection.z = max(fma(primitiveState.lastIntersection.z-diffOffset, 1.f/dirlen, 1e-4f), 1e-4f);
}
