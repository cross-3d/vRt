
// default definitions

#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif


// global memory stack pages (256-bit)
//#ifdef ENABLE_AMD_INT16
//const int16_t localStackSize = 8s, pageCount = 4s, computedStackSize = localStackSize*pageCount; 
//#else 
const lowp int localStackSize = 8, pageCount = 4, computedStackSize = localStackSize*pageCount; 
//#endif

const highp int max_iteraction = 8192;
layout ( std430, binding = _CACHE_BINDING, set = 0 ) coherent buffer VT_PAGE_SYSTEM { int pages[][8]; };


// BVH traversing state
#define _cacheID gl_GlobalInvocationID.x
struct BvhTraverseState {
    //int idx, defTriangleID, stackPtr, cacheID, pageID, maxTriangles;
    int idx, defTriangleID, maxTriangles; 
//#ifdef ENABLE_AMD_INT16
//    int16_t stackPtr, pageID;
//#else
    lowp int stackPtr, pageID;
//#endif
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
    int idx = --traverseState.stackPtr, rsl = idx >= 0 ? lstack[idx] : -1; traverseState.stackPtr = max(traverseState.stackPtr, 0); return rsl;
};

void storeStack(in int rsl) {
    [[flatten]] if (traverseState.stackPtr >= localStackSize && traverseState.pageID < pageCount) {
        pages[_cacheID*pageCount + (traverseState.pageID++)] = lstack; traverseState.stackPtr = 0;
    }
    int idx = traverseState.stackPtr++; 
    [[flatten]] if (idx < localStackSize) lstack[idx] = rsl; traverseState.stackPtr = min(traverseState.stackPtr, localStackSize);
};


// triangle intersection, when it found
void doIntersection(in bool isvalid) {
    isvalid = isvalid && traverseState.defTriangleID > 0 && traverseState.defTriangleID <= traverseState.maxTriangles;
    IFANY (isvalid) {
        vec2 uv = vec2(0.f.xx); float d = 
#ifdef VRT_USE_FAST_INTERSECTION
            intersectTriangle(primitiveState.orig, primitiveState.dir, traverseState.defTriangleID-1, uv.xy, isvalid, primitiveState.lastIntersection.z);
#else
            intersectTriangle(primitiveState.orig, primitiveState.iM, primitiveState.axis, traverseState.defTriangleID-1, uv.xy, isvalid);
#endif
#define nearhit (primitiveState.lastIntersection.z)

        [[flatten]] if (d <= nearhit && d <= N_INFINITY && isvalid) {
            [[flatten]] if (abs(primitiveState.lastIntersection.z-d) > 0.f || traverseState.defTriangleID > floatBitsToInt(primitiveState.lastIntersection.w)) {
                primitiveState.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defTriangleID));
            };
        };
    }; traverseState.defTriangleID=0;
};


// corrections of box intersection
const bvec3 bsgn = false.xxx;

// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void traverseBvh2(in bool valid, in int eht, in vec3 orig, in vec2 pdir) {

    // relative origin and vector
    vec4 torig = -divW(mult4( bvhBlock.projection, vec4(orig, 1.0f))), torigTo = divW(mult4( bvhBlock.projection, vec4(orig, 1.0f) + vec4(dcts(pdir.xy), 0.f))), tdir = torigTo+torig;
    torig = -uniteBox(-torig), torigTo = uniteBox(torigTo), tdir = torigTo+torig;
    
    // different length of box space and global space
    const float phslen = length(tdir);
    //const float dirlen = phslen, invlen = 1.f/precIssue(dirlen);
    const float dirlen = 1.f, invlen = 1.f/precIssue(dirlen);
    const vec4 direct = tdir * invlen, dirproj = 1.f / precIssue(direct);

    // limitation of distance
    //#define bsgn traverseState.boxSide.xyz
    //bsgn = greaterThan(direct.xyz, 0.f.xxx);

    // pre-calculate for triangle intersections
#ifdef VRT_USE_FAST_INTERSECTION
    primitiveState.dir = direct;
#else
    // calculate longest axis
    primitiveState.axis = 2; {
         vec3 drs = abs(direct);
         if (drs.y >= drs.x && drs.y > drs.z) primitiveState.axis = 1;
         if (drs.x >= drs.z && drs.x > drs.y) primitiveState.axis = 0;
         if (drs.z >= drs.y && drs.z > drs.x) primitiveState.axis = 2;
    }

    // calculate affine matrices
    const vec4 vm = vec4(-direct, 1.f) / precIssue(primitiveState.axis == 0 ? direct.x : (primitiveState.axis == 1 ? direct.y : direct.z));
    primitiveState.iM = transpose(mat3(
        primitiveState.axis == 0 ? vm.wyz : vec3(1.f,0.f,0.f),
        primitiveState.axis == 1 ? vm.xwz : vec3(0.f,1.f,0.f),
        primitiveState.axis == 2 ? vm.xyw : vec3(0.f,0.f,1.f)
    ));
#endif

    // test intersection with main box
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    const   vec3 interm = bvhBlock.sceneMax.xyz - bvhBlock.sceneMin.xyz;
    const   vec2 bside2 = vec2(-1.f, 1.f);
    const mat3x2 bndsf2 = mat3x2( fma(InZero,interm.x,1.f)*bside2, fma(InZero,interm.y,1.f)*bside2, fma(InZero,interm.z,1.f)*bside2 );
    //const mat3x2 bndsf2 = transpose(mat2x3(bvhBlock.sceneMin.xyz, bvhBlock.sceneMax.xyz));
    const int entry = (valid ? BVH_ENTRY : -1);

    // initial traversing state
    //traverseState.idx = entry, traverseState.idx = nfe.x >= N_INFINITY ? -1 : traverseState.idx; // unable to intersect the root box 
    traverseState.idx = intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe) ? entry : -1, traverseState.idx = nfe.x > N_INFINITY ? -1 : traverseState.idx;
    traverseState.stackPtr = 0, traverseState.pageID = 0, traverseState.defTriangleID = 0; float diffOffset = min(-nfe.x, 0.f);
    traverseState.minusOrig = fvec4_(fma(fvec4_(torig), fvec4_(dirproj), fvec4_(diffOffset.xxxx)));
    traverseState.directInv = fvec4_(dirproj);
    
    // initial intersection state
    primitiveState.orig = fma(direct, diffOffset.xxxx, torig);
    primitiveState.lastIntersection = eht >= 0 ? hits[eht].uvt : vec4(0.f.xx, INFINITY, FINT_ZERO), primitiveState.lastIntersection.z = min(primitiveState.lastIntersection.z, INFINITY);
    primitiveState.lastIntersection.z = fma(primitiveState.lastIntersection.z, dirlen, diffOffset);

    // two loop based BVH traversing
    [[dependency_infinite]] for (int hi=0;hi<max_iteraction;hi++) {
        [[flatten]] if (traverseState.idx >= 0 && traverseState.defTriangleID <= 0) {
        { [[dependency_infinite]] for (;hi<max_iteraction;hi++) { bool _continue = false;
            //const NTYPE_ bvhNode = bvhNodes[traverseState.idx]; // each full node have 64 bytes
            #define bvhNode bvhNodes[traverseState.idx] // ref directly
            //#define cnode (bvhNode.meta.xy) // reuse already got
            
            ivec2 cnode = traverseState.idx >= 0 ? bvhNode.meta.xy : (0).xx;
            [[flatten]] if (isLeaf(cnode.xy)) { traverseState.defTriangleID = cnode.x; } // if leaf, defer for intersection 
            else { // if not leaf, intersect with nodes
                //const fmat3x4_ bbox2x = fmat3x4_(bvhNode.cbox[0], bvhNode.cbox[1], bvhNode.cbox[2]);
                #define bbox2x fmat3x4_(bvhNode.cbox[0],bvhNode.cbox[1],bvhNode.cbox[2]) // use same memory
                lowp bvec2_ childIntersect = bvec2_(traverseState.idx >= 0) & intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe);

                // found simular technique in http://www.sci.utah.edu/~wald/Publications/2018/nexthit-pgv18.pdf
                // but we came up in past years, so sorts of patents may failure 
                // also, they uses hit queue, but it can very overload stacks, so saving only indices...
                childIntersect &= bvec2_(lessThanEqual(nfe.xy, primitiveState.lastIntersection.zz)); // it increase FPS by filtering nodes by first triangle intersection
                
                // 
                int fmask = int((childIntersect.y<<1u)|childIntersect.x);
                [[flatten]] if (fmask > 0) {
                    int primary = -1, secondary = -1;
                    [[flatten]] if (fmask == 3) { fmask &= nfe.x<=nfe.y ? 1 : 2, secondary = cnode.x^(fmask>>1); }; // if both has intersection
                    primary = cnode.x^(fmask&1);
                    
                    {
                        // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                        // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                        //#define snode (bvhNodes[secondary].meta.xy) // use reference only
                        const ivec2 snode = bvhNodes[secondary].meta.xy;
                        [[flatten]] if (secondary > 0 && isLeaf(snode)) { traverseState.defTriangleID = snode.x; secondary = -1; } else 
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
        [[flatten]] IFANY (traverseState.defTriangleID > 0) { doIntersection( true ); } // if has triangle, do intersection
        [[flatten]] if (traverseState.idx <= 0) { break; } // if no to traversing - breaking
    };

    // correction of hit distance
    primitiveState.lastIntersection.z = fma(primitiveState.lastIntersection.z, invlen, -diffOffset*invlen);
    primitiveState.lastIntersection.z = min(primitiveState.lastIntersection.z, INFINITY); // clamp by infinite
};
