
// default definitions

#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif


#ifdef ENABLE_VEGA_INSTRUCTION_SET
  const  lowp  int localStackSize = 8, pageCount = 4, computedStackSize = localStackSize*pageCount; // 256-bit global memory stack pages
#else
  const  lowp  int localStackSize = 4, pageCount = 8, computedStackSize = localStackSize*pageCount; // 128-bit capable (minor GPU, GDDR6 two-channels)
#endif
  const highp uint maxIterations  = 8192;


layout ( binding = _CACHE_BINDING, set = 0, std430 ) coherent buffer VT_PAGE_SYSTEM { int pages[][localStackSize]; };


// BVH traversing state
#define _cacheID gl_GlobalInvocationID.x
struct BvhTraverseState {
         int idx, defTriangleID, maxTriangles; float diffOffset;
    lowp int stackPtr, pageID;
    fvec4_ directInv, minusOrig;
} traverseState;


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

#define sidx traverseState.stackPtr
void loadStack(inout int rsl) {
    rsl = (--sidx) >= 0 ? lstack[sidx] : -1; traverseState.stackPtr = max(traverseState.stackPtr, 0);
    [[flatten]] if (traverseState.stackPtr <= 0 && traverseState.pageID > 0) { // make store/load deferred 
        lstack = pages[_cacheID*pageCount + (--traverseState.pageID)]; traverseState.stackPtr = localStackSize;
    };
};

void storeStack(in int rsl) {
    [[flatten]] if (traverseState.stackPtr >= localStackSize && traverseState.pageID < pageCount) { // make store/load deferred 
        pages[_cacheID*pageCount + (traverseState.pageID++)] = lstack; traverseState.stackPtr = 0;
    };
    [[flatten]] if (sidx < localStackSize) lstack[sidx++] = rsl; traverseState.stackPtr = min(traverseState.stackPtr, localStackSize);
};


//const float fpInner = 128.f*SFN, fpOne = 1.f;//SFO;
#ifndef fpInner
const float fpInner = 0.0000152587890625, fpOne = SFO;
#endif

// triangle intersection, when it found
void doIntersection(in bool isvalid, in float dlen) {
    isvalid = isvalid && traverseState.defTriangleID > 0 && traverseState.defTriangleID <= traverseState.maxTriangles;
    IFANY (isvalid) {
        vec2 uv = vec2(0.f.xx); const float nearT = fma(primitiveState.lastIntersection.z,fpOne,fpInner), d = 
#ifdef VRT_USE_FAST_INTERSECTION
            intersectTriangle(primitiveState.orig, primitiveState.dir, traverseState.defTriangleID-1, uv.xy, isvalid, nearT);
#else
            intersectTriangle(primitiveState.orig, primitiveState.iM, primitiveState.axis, traverseState.defTriangleID-1, uv.xy, isvalid);
#endif

        const float tdiff = nearT-d, tmax = 0.f;
        [[flatten]] if (tdiff >= -tmax && d < N_INFINITY && isvalid) {
            [[flatten]] if (abs(tdiff) > tmax || traverseState.defTriangleID > floatBitsToInt(primitiveState.lastIntersection.w)) {
                primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defTriangleID));
            };
        };
    }; traverseState.defTriangleID=0;
};

// corrections of box intersection
const bvec4 bsgn = false.xxxx;
const float dirlen = 1.f, invlen = 1.f, bsize = 1.f;

// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void resetEntry(in bool valid) { traverseState.idx = (valid ? BVH_ENTRY : -1), traverseState.stackPtr = 0, traverseState.pageID = 0, traverseState.defTriangleID = 0; };
void initTraversing(in bool valid, in int eht, in vec3 orig, in dirtype_t pdir) {
    
    // relative origin and vector
    vec4 torig = -divW(mult4( bvhBlock.projection, vec4(orig, 1.0f))), torigTo = divW(mult4( bvhBlock.projection, vec4(orig, 1.0f) + vec4(dcts(pdir), 0.f))), tdir = torigTo+torig;
    torig = -uniteBox(-torig), torigTo = uniteBox(torigTo), tdir = torigTo+torig;

    // different length of box space and global space
    //const float dirlen = length(tdir), invlen  = 1.f / precIssue(dirlen);
      const  vec4 direct = tdir *invlen, dirproj = 1.f / precIssue(direct);

    // test intersection with main box
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    const   vec3 interm = fma(fpInner.xxx, 2.f.xxx / (bvhBlock.sceneMax.xyz - bvhBlock.sceneMin.xyz), 1.f.xxx);
    const   vec2 bside2 = vec2(-fpOne, fpOne);
    const mat3x2 bndsf2 = mat3x2( bside2*interm.x, bside2*interm.y, bside2*interm.z );

    // initial traversing state
    resetEntry(valid); //traverseState.bsgn = bvec3_(greaterThanEqual(dirproj.xyz, 0.f.xxx)); // sign for box intersections
    [[flatten]] if (!intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe)) { traverseState.idx = -1; };
    [[flatten]] if (eht.x >= 0) primitiveState.lastIntersection = hits[eht].uvt;

    // traversing inputs
    traverseState.diffOffset = min(-nfe.x, 0.f);
    traverseState.directInv = fvec4_(dirproj);
    traverseState.minusOrig = fvec4_(vec4(fma(fvec4_(torig), traverseState.directInv, ftype_(traverseState.diffOffset).xxxx)));

    // intersection inputs
    primitiveState.dir = direct, primitiveState.orig = fma(direct, traverseState.diffOffset.xxxx, torig);
};


void traverseBVH2( in bool reset, in bool valid ) {
    primitiveState.lastIntersection.z = fma(min(primitiveState.lastIntersection.z, INFINITY), dirlen, traverseState.diffOffset);
    [[flatten]] if (reset) resetEntry(valid);
    [[flatten]] if (traverseState.maxTriangles <= 0 || primitiveState.lastIntersection.z < 0.f) { traverseState.idx = -1; };

    // two loop based BVH traversing
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    [[dependency_infinite]] for (uint hi=0;hi<maxIterations;hi++) {
        [[flatten]] if (traverseState.idx >= 0 && traverseState.defTriangleID <= 0) {
        { [[dependency_infinite]] for (;hi<maxIterations;hi++) { bool _continue = false;
            //const NTYPE_ bvhNode = bvhNodes[traverseState.idx]; // each full node have 64 bytes
            #define bvhNode bvhNodes[traverseState.idx] // ref directly
            //#define cnode (bvhNode.meta.xy) // reuse already got

            const ivec2 cnode = traverseState.idx >= 0 ? bvhNode.meta.xy : (0).xx;
            [[flatten]] if (isLeaf(cnode.xy)) { traverseState.defTriangleID = cnode.x; } // if leaf, defer for intersection 
            else { // if not leaf, intersect with nodes
                //const fmat3x4_ bbox2x = fmat3x4_(bvhNode.cbox[0], bvhNode.cbox[1], bvhNode.cbox[2]);
                #define bbox2x bvhNode.cbox // use same memory
                bvec2_ childIntersect = (bool(cnode.x&1) && cnode.x>0 && traverseState.idx>=0) ? bvec2_(intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe)) : false_.xx;

                // found simular technique in http://www.sci.utah.edu/~wald/Publications/2018/nexthit-pgv18.pdf
                // but we came up in past years, so sorts of patents may failure 
                // also, they uses hit queue, but it can very overload stacks, so saving only indices...
                childIntersect &= bvec2_(lessThanEqual(nfe.xy, fma(primitiveState.lastIntersection.z,fpOne,fpInner).xx)); // it increase FPS by filtering nodes by first triangle intersection

                // 
                bool_ fmask = childIntersect.x|(childIntersect.y<<true_);
                [[flatten]] if (fmask > 0) {
                    int primary = -1, secondary = -1;
                    [[flatten]] if (fmask == 3) { fmask &= true_<<bool_(nfe.x>nfe.y); secondary = cnode.x^int(fmask>>1u); }; // if both has intersection
                    primary = cnode.x^int(fmask&1u);

                    // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                    // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                    //#define snode (bvhNodes[secondary].meta.xy) // use reference only
                    [[flatten]] if (secondary > 0) {
                        const ivec2 snode = bvhNodes[secondary].meta.xy;
                        [[flatten]] if (isLeaf(snode)) { traverseState.defTriangleID = snode.x; secondary = -1; } else 
                        [[flatten]] if (secondary > 0) storeStack(secondary);
                    };

                    // set traversing node id
                    traverseState.idx = primary, _continue = true;
                }
            };

            // if all threads had intersection, or does not given any results, break for processing
            [[flatten]] if (!_continue && traverseState.idx > 0) { loadStack(traverseState.idx); } // load from stack 
            [[flatten]] IFANY (traverseState.defTriangleID > 0 || traverseState.idx <= 0) { break; } // 
        }}};

        // every-step solving 
        [[flatten]] IFANY (traverseState.defTriangleID > 0) { doIntersection( true, bsize ); } // if has triangle, do intersection
        [[flatten]] if (traverseState.idx <= 0) { break; } // if no to traversing - breaking
    };

    // correction of hit distance
    primitiveState.lastIntersection.z = min(fma(primitiveState.lastIntersection.z, invlen, -traverseState.diffOffset*invlen), INFINITY);
};
