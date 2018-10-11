
// default definitions

#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif


#ifndef fpInner
  const float fpInner = 0.0000152587890625f, fpOne = 1.f;
#endif

#ifdef ENABLE_VEGA_INSTRUCTION_SET
  const  lowp  int localStackSize = 8, pageCount = 4; // 256-bit global memory stack pages
#else
  const  lowp  int localStackSize = 4, pageCount = 8; // 128-bit capable (minor GPU, GDDR6 two-channels)
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
     vec4 lastIntersection; // used from previosly only 
    ivec4 raydata; // with rayID
} primitiveState;

#define RAY_ID primitiveState.raydata[0]



// stack system of current BVH traverser
shared int localStack[WORK_SIZE][localStackSize];
#define lstack localStack[Local_Idx]
#define sidx  traverseState.stackPtr

void loadStack(inout int rsl) {
    [[flatten]] if ((--sidx) >= 0) rsl = lstack[sidx];
    [[flatten]] if (traverseState.stackPtr <= 0 && traverseState.pageID > 0) { // make store/load deferred 
        lstack = pages[_cacheID*pageCount + (--traverseState.pageID)]; traverseState.stackPtr = localStackSize;
    }; traverseState.stackPtr = max(traverseState.stackPtr, 0);
};

void storeStack(in int rsl) {
    [[flatten]] if (traverseState.stackPtr >= localStackSize && traverseState.pageID < pageCount) { // make store/load deferred 
        pages[_cacheID*pageCount + (traverseState.pageID++)] = lstack; traverseState.stackPtr = 0;
    };
    [[flatten]] if (sidx < localStackSize) lstack[sidx++] = rsl; 
    traverseState.stackPtr = min(traverseState.stackPtr, localStackSize);
};


// TODO: add task pushing
int pushTask(in int instanceID, in int rayID){
    //int taskID = 
};

// 
void doIntersection(in bool isvalid, in float dlen) {
    // TODO: need to correct naming 
    isvalid = isvalid && traverseState.defTriangleID > 0 && traverseState.defTriangleID <= traverseState.maxTriangles;
    IFANY (isvalid) {

        [[flatten]] if (isvalid) pushTask(traverseState.defTriangleID, RAY_ID); // push a task to bottom level traversing 

    }; traverseState.defTriangleID=0;
};


// corrections of box intersection
const bvec4 bsgn = false.xxxx;
const 
float dirlen = 1.f, invlen = 1.f, bsize = 1.f;


// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void resetEntry(in bool valid) { traverseState.idx = (valid ? BVH_ENTRY : -1), traverseState.stackPtr = 0, traverseState.pageID = 0, traverseState.defTriangleID = 0; };
void initTraversing( in bool valid, in int eht, in vec3 orig, in dirtype_t pdir, in int rayID ) {
    [[flatten]] if (eht.x >= 0) primitiveState.lastIntersection = hits[eht].uvt;
    RAY_ID = rayID; // for pushing a traversing tasks

    // relative origin and vector ( also, preparing mat3x4 support ) 
    // in task-based traversing will have universal transformation for BVH traversing and transforming in dimensions 
    const vec4 torig = -uniteBox(vec4(mult4(bvhBlock.transform, vec4(orig, 1.f)).xyz, 1.f)), torigTo = uniteBox(vec4(mult4(bvhBlock.transform, vec4(orig, 1.f) + vec4(dcts(pdir).xyz, 0.f)).xyz, 1.f)), tdir = torigTo+torig;
    const vec4 dirct = tdir*invlen, dirproj = 1.f / precIssue(dirct);
    primitiveState.dir = primitiveState.orig = dirct;

    // test intersection with main box
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    const   vec3 interm = fma(fpInner.xxxx, 2.f / (bvhBlock.sceneMax - bvhBlock.sceneMin), 1.f.xxxx).xyz;
    const   vec2 bside2 = vec2(-fpOne, fpOne);
    const mat3x2 bndsf2 = mat3x2( bside2*interm.x, bside2*interm.y, bside2*interm.z );

    // initial traversing state
    valid = valid && intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe), resetEntry(valid);

    // traversing inputs
    traverseState.diffOffset = min(-nfe.x, 0.f);
    traverseState.directInv = fvec4_(dirproj), traverseState.minusOrig = fvec4_(vec4(fma(fvec4_(torig), traverseState.directInv, ftype_(traverseState.diffOffset).xxxx)));
    primitiveState.orig = fma(primitiveState.orig, traverseState.diffOffset.xxxx, torig);
};


// TODO: unify traversing codes 
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
            #define bvhNode bvhNodes[traverseState.idx]
            const ivec2 cnode = traverseState.idx >= 0 ? bvhNode.meta.xy : (0).xx;
            [[flatten]] if (isLeaf(cnode.xy)) { traverseState.defTriangleID = cnode.x; } // if leaf, defer for intersection 
            else { // if not leaf, intersect with nodes
                //const fmat3x4_ bbox2x = fmat3x4_(bvhNode.cbox[0], bvhNode.cbox[1], bvhNode.cbox[2]);

#ifdef EXPERIMENTAL_UNORM16_BVH
                #define bbox2x fvec4_[3](\
                    fvec4_(traverseState.idx>=0?unpackSnorm4x16(bvhNode.cbox[0]):0.f.xxxx),\
                    fvec4_(traverseState.idx>=0?unpackSnorm4x16(bvhNode.cbox[1]):0.f.xxxx),\
                    fvec4_(traverseState.idx>=0?unpackSnorm4x16(bvhNode.cbox[2]):0.f.xxxx)\
                )
#else
                #define bbox2x (traverseState.idx>=0?bvhNode.cbox:fvec4_[3](0.f.xxxx,0.f.xxxx,0.f.xxxx)) // use same memory
#endif

                pbvec2_ childIntersect = bool(cnode.x&1) ? intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe) : false2_;

                // found simular technique in http://www.sci.utah.edu/~wald/Publications/2018/nexthit-pgv18.pdf
                // but we came up in past years, so sorts of patents may failure 
                // also, they uses hit queue, but it can very overload stacks, so saving only indices...
                childIntersect &= binarize(lessThanEqual(nfe.xy, fma(primitiveState.lastIntersection.z,fpOne,fpInner).xx)); // it increase FPS by filtering nodes by first triangle intersection

                // 
                pbool_ fmask = pl_x(childIntersect)|(pl_y(childIntersect)<<true_);
                [[flatten]] if (fmask > 0) { _continue = true;
                    int secondary = -1; 
                    [[flatten]] if (fmask == 3) { fmask &= true_<<pbool_(nfe.x>nfe.y); secondary = cnode.x^int(fmask>>1u); }; // if both has intersection
                    traverseState.idx = cnode.x^int(fmask&1u); // set traversing node id

                    // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                    // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                    //#define snode (bvhNodes[secondary].meta.xy) // use reference only
                    [[flatten]] if (secondary > 0) {
                        const ivec2 snode = bvhNodes[secondary].meta.xy;
                        [[flatten]] if (isLeaf(snode)) { traverseState.defTriangleID = snode.x; secondary = -1; } else 
                        [[flatten]] if (secondary > 0) storeStack(secondary);
                    };
                };
            };

            // if all threads had intersection, or does not given any results, break for processing
            [[flatten]] if ( !_continue && traverseState.idx > 0 ) { traverseState.idx = -1, loadStack(traverseState.idx); } // load from stack 
            [[flatten]] IFANY (traverseState.defTriangleID > 0 || traverseState.idx <= 0) { break; } // 
        }}};

        // every-step solving 
        [[flatten]] IFANY (traverseState.defTriangleID > 0) { doIntersection( true, bsize ); } // instance task pushing 
        [[flatten]] if (traverseState.idx <= 0) { break; } // if no to traversing - breaking
    };

    // correction of hit distance
    primitiveState.lastIntersection.z = min(fma(primitiveState.lastIntersection.z, invlen, -traverseState.diffOffset*invlen), INFINITY);
};
