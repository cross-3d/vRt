
// default definitions

#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif


// global memory stack pages (256-bit)
//const  lowp int localStackSize = 8, pageCount = 4, computedStackSize = localStackSize*pageCount;
  const  lowp  int localStackSize = 4, pageCount = 8, computedStackSize = localStackSize*pageCount; // 128-bit capable (minor GPU, GDDR6 two-channels)
  const highp uint  maxIterations = 8192;

layout ( std430, binding = _CACHE_BINDING, set = 0 ) coherent buffer VT_PAGE_SYSTEM { int pages[][localStackSize]; };


// BVH traversing state
#define _cacheID gl_GlobalInvocationID.x
struct BvhTraverseState {
         int idx, defTriangleID, maxTriangles; 
    lowp int stackPtr, pageID;
    fvec4_ minusOrig, directInv; // vec4 of 32-bits
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

int loadStack() {
    [[flatten]] if (traverseState.stackPtr <= 0 && traverseState.pageID > 0) { 
        lstack = pages[_cacheID*pageCount + (--traverseState.pageID)]; traverseState.stackPtr = localStackSize; 
    };
    const int idx = --traverseState.stackPtr, rsl = idx >= 0 ? lstack[idx] : -1; traverseState.stackPtr = max(traverseState.stackPtr, 0); return rsl;
};

void storeStack(in int rsl) {
    [[flatten]] if (traverseState.stackPtr >= localStackSize && traverseState.pageID < pageCount) {
        pages[_cacheID*pageCount + (traverseState.pageID++)] = lstack; traverseState.stackPtr = 0;
    }
    const int idx = traverseState.stackPtr++; 
    [[flatten]] if (idx < localStackSize) lstack[idx] = rsl; traverseState.stackPtr = min(traverseState.stackPtr, localStackSize);
};


#ifndef fpInner
#define fpInner (128.f*SFN) //0.00000011920928955078125f
#endif

// triangle intersection, when it found
void doIntersection(in bool isvalid, in float dlen) {
    isvalid = isvalid && traverseState.defTriangleID > 0 && traverseState.defTriangleID <= traverseState.maxTriangles;
    IFANY (isvalid) { 
        vec2 uv = vec2(0.f.xx); const float d = 
#ifdef VRT_USE_FAST_INTERSECTION
            intersectTriangle(primitiveState.orig, primitiveState.dir, traverseState.defTriangleID-1, uv.xy, isvalid, INFINITY);
#else
            intersectTriangle(primitiveState.orig, primitiveState.iM, primitiveState.axis, traverseState.defTriangleID-1, uv.xy, isvalid);
#endif

        const float tdiff = (primitiveState.lastIntersection.z-d), tmax = fpInner;// d * fpInner;
        [[flatten]] if (tdiff >= -tmax && d <= N_INFINITY && isvalid) {
            [[flatten]] if (abs(tdiff) > tmax || traverseState.defTriangleID > floatBitsToInt(primitiveState.lastIntersection.w)) {
                primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defTriangleID));
            };
        };
    }; traverseState.defTriangleID=0;
};

// corrections of box intersection
const bvec3 bsgn = false.xxx;
const float dirlen = 1.f, invlen = 1.f, bsize = 1.f;

// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void traverseBvh2(in bool valid, in int eht, in vec3 orig, in vec2 pdir) {

    // relative origin and vector
    vec4 torig = -divW(mult4( bvhBlock.projection, vec4(orig, 1.0f))), torigTo = divW(mult4( bvhBlock.projection, vec4(orig, 1.0f) + vec4(dcts(pdir.xy), 0.f))), tdir = torigTo+torig;
    torig = -uniteBox(-torig), torigTo = uniteBox(torigTo), tdir = torigTo+torig;

    // different length of box space and global space
    //const float dirlen  = length(tdir), invlen = 1.f / precIssue(dirlen);
    const vec4 direct = tdir * invlen, dirproj = 1.f / precIssue(direct);

/*
#ifdef VRT_USE_FAST_INTERSECTION
    primitiveState.dir = direct;
#else
    // calculate longest axis
    primitiveState.axis = 2; {
         vec3 drs = abs(direct);
         if (drs.x >= drs.z && drs.x > drs.y) primitiveState.axis = 0;
         if (drs.y >= drs.x && drs.y > drs.z) primitiveState.axis = 1;
         if (drs.z >= drs.y && drs.z > drs.x) primitiveState.axis = 2;
    };

    // calculate affine matrices
    const vec4 vm = vec4(-direct, 1.f) / precIssue(primitiveState.axis == 0 ? direct.x : (primitiveState.axis == 1 ? direct.y : direct.z));
    primitiveState.iM = transpose(mat3(
        primitiveState.axis == 0 ? vm.wyz : vec3(1.f,0.f,0.f),
        primitiveState.axis == 1 ? vm.xwz : vec3(0.f,1.f,0.f),
        primitiveState.axis == 2 ? vm.xyw : vec3(0.f,0.f,1.f)
    ));
#endif
*/

    // test intersection with main box
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    const   vec3 interm = fma(fpInner.xxx, 4.f.xxx / precIssue(bvhBlock.sceneMax.xyz - bvhBlock.sceneMin.xyz), 1.f.xxx);
    const   vec2 bside2 = vec2(-SFO, SFO);
    const mat3x2 bndsf2 = mat3x2( bside2*interm.x, bside2*interm.y, bside2*interm.z );
    const int entry = (valid ? BVH_ENTRY : -1);

    // initial traversing state
    //traverseState.idx = entry, traverseState.idx = nfe.x >= N_INFINITY ? -1 : traverseState.idx; // unable to intersect the root box 
    traverseState.idx = intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe) ? entry : -1, traverseState.idx = nfe.x > N_INFINITY ? -1 : traverseState.idx;
    traverseState.stackPtr = 0, traverseState.pageID = 0, traverseState.defTriangleID = 0; float diffOffset = min(-nfe.x, 0.f);
    traverseState.directInv = fvec4_(dirproj), traverseState.minusOrig = fvec4_(fma(fvec4_(torig), fvec4_(dirproj), fvec4_(diffOffset.xxxx)));

    // initial intersection state
    [[flatten]] if (eht >= 0) primitiveState.lastIntersection = hits[eht].uvt;
    primitiveState.lastIntersection.z = fma(min(primitiveState.lastIntersection.z, INFINITY), dirlen, diffOffset);
    primitiveState.dir = direct, primitiveState.orig = fma(direct, diffOffset.xxxx, torig);

    // two loop based BVH traversing
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
                #define bbox2x fmat3x4_(bvhNode.cbox[0],bvhNode.cbox[1],bvhNode.cbox[2]) // use same memory
                lowp bvec2_ childIntersect = bvec2_(traverseState.idx >= 0) & intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe);
                childIntersect &= (cnode.x&1).xx; // validate BVH node

                // found simular technique in http://www.sci.utah.edu/~wald/Publications/2018/nexthit-pgv18.pdf
                // but we came up in past years, so sorts of patents may failure 
                // also, they uses hit queue, but it can very overload stacks, so saving only indices...
                childIntersect &= bvec2_(lessThanEqual(nfe.xy, primitiveState.lastIntersection.zz*SFO)); // it increase FPS by filtering nodes by first triangle intersection

                // 
                int fmask = int((childIntersect.y<<1u)|childIntersect.x);
                [[flatten]] if (fmask > 0) {
                    int primary = -1, secondary = -1;
                    [[flatten]] if (fmask == 3) { fmask &= nfe.x<=nfe.y ? 1 : 2, secondary = cnode.x^(fmask>>1); }; // if both has intersection
                    primary = cnode.x^(fmask&1);

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
            }

            // if all threads had intersection, or does not given any results, break for processing
            [[flatten]] if (!_continue) { traverseState.idx = loadStack(); } // load from stack 
            [[flatten]] IFANY (traverseState.defTriangleID > 0 || traverseState.idx <= 0) { break; } // 
        }}};

        // every-step solving 
        [[flatten]] IFANY (traverseState.defTriangleID > 0) { doIntersection( true, bsize ); } // if has triangle, do intersection
        [[flatten]] if (traverseState.idx <= 0) { break; } // if no to traversing - breaking
    };

    // correction of hit distance
    primitiveState.lastIntersection.z = fma(primitiveState.lastIntersection.z, invlen, -diffOffset*invlen);
    primitiveState.lastIntersection.z = min(primitiveState.lastIntersection.z, INFINITY); // clamp by infinite
};
