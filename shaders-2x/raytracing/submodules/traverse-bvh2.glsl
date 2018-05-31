
// default definitions

#ifndef _CACHE_BINDING
#define _CACHE_BINDING 4
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE ElectedRay
#endif


const int max_iteraction = 8192;
//const int stackPageCount = 8;
//const int localStackSize = 4;
const int stackPageCount = 4;
const int localStackSize = 8;

// dedicated BVH stack
//struct NodeCache { ivec4 stackPages[stackPageCount]; };
//layout ( std430, binding = _CACHE_BINDING, set = 0 ) buffer nodeCache { NodeCache nodeCache[]; };

layout ( rgba32i, binding = _CACHE_BINDING, set = 0 )  uniform iimageBuffer texelPages;


// 128-bit payload
int stackPtr = 0, pagePtr = 0, cacheID = 0, _r0 = -1;


#ifndef USE_STACKLESS_BVH
//#define lstack localStack[Local_Idx]
shared ivec4 localStack[WORK_SIZE][2];
#define lstack localStack[Local_Idx]
//ivec4 lstack = ivec4(-1,-1,-1,-1);

int loadStack(){
    // load previous stack page
    if ((--stackPtr) < 0) {
        int page = --pagePtr;
        if (page >= 0 && page < stackPageCount) {
            stackPtr = localStackSize-1;
            lstack = ivec4[2](imageLoad(texelPages, (cacheID*stackPageCount + page)*2+0), imageLoad(texelPages, (cacheID*stackPageCount + page)*2+1));
        }
    }

    // fast-stack
    //int val = exchange(lstack.x, -1); lstack = lstack.yzwx;
    int val = exchange(lstack[0].x, -1); lstack = ivec4[2](ivec4(lstack[0].yzw, lstack[1].x), ivec4(lstack[1].yzw, lstack[0].x));
    return val;
}

void storeStack(in int val){
    // store stack to global page, and empty list
    if ((stackPtr++) >= localStackSize) {
        int page = pagePtr++;
        if (page >= 0 && page < stackPageCount) { 
            stackPtr = 1;
            imageStore(texelPages, (cacheID*stackPageCount + page)*2+0, lstack[0]);
            imageStore(texelPages, (cacheID*stackPageCount + page)*2+1, lstack[1]);
        }
    }

    // fast-stack
    //lstack = lstack.wxyz; lstack.x = val;
    lstack = ivec4[2](ivec4(lstack[1].w, lstack[0].xyz), ivec4(lstack[0].w, lstack[1].xyz)); lstack[0].x = val;
}

bool stackIsFull() { return stackPtr >= localStackSize && pagePtr >= stackPageCount; }
bool stackIsEmpty() { return stackPtr <= 0 && pagePtr < 0; }
#endif



//shared _RAY_TYPE rayCache[WORK_SIZE];
//#define currentRayTmp rayCache[Local_Idx]
_RAY_TYPE currentRayTmp;

struct BvhTraverseState {
    int idx, defTriangleID;
    float distMult, diffOffset;

#ifdef USE_STACKLESS_BVH
    uint64_t bitStack, bitStack2;
#endif
} traverseState;

struct GeometrySpace {
    vec4 lastIntersection;
    //vec4 dir;
    int axis; mat3 iM;
} geometrySpace;

struct BVHSpace {
    fvec4_ minusOrig, directInv; bvec4_ boxSide, _3;
    float cutOut, _0, _1, _2;
} bvhSpace;



void doIntersection() {
    bool_ near = bool_(traverseState.defTriangleID >= 0);
    vec2 uv = vec2(0.f.xx);
    float d = intersectTriangle(currentRayTmp.origin.xyz, geometrySpace.iM, geometrySpace.axis, traverseState.defTriangleID, uv.xy, bool(near.x));
    //float d = intersectTriangle(currentRayTmp.origin.xyz, geometrySpace.dir.xyz, traverseState.defTriangleID, uv.xy, bool(near.x));
    float nearhit = geometrySpace.lastIntersection.z;

    [[flatten]]
    IF (lessF(d, nearhit)) { bvhSpace.cutOut = d * traverseState.distMult - traverseState.diffOffset; }
    
    // validate hit 
    near &= lessF(d, INFINITY) & lessEqualF(d, nearhit);

    [[flatten]]
    IF (near.x) geometrySpace.lastIntersection = vec4(uv.xy, d.x, intBitsToFloat(traverseState.defTriangleID+1));

    // reset triangle ID 
    traverseState.defTriangleID = -1;
}

void traverseBvh2(in bool_ valid, inout _RAY_TYPE rayIn) {
    currentRayTmp = rayIn;
    vec3 origin = currentRayTmp.origin.xyz;
    vec3 direct = dcts(currentRayTmp.cdirect.xy);
    int eht = floatBitsToInt(currentRayTmp.origin.w)-1;

    // reset stack
    stackPtr = 0, pagePtr = 0;
    lstack = ivec4[2]((-1).xxxx, (-1).xxxx);

    // test constants
    vec3 
        torig = -divW(mult4(GEOMETRY_BLOCK geometryUniform.transform, vec4(origin, 1.0f))).xyz,
        torigTo = divW(mult4(GEOMETRY_BLOCK geometryUniform.transform, vec4(origin+direct, 1.0f))).xyz,
        dirproj = torigTo+torig;

    // get vector length and normalize
    float dirlen = length(dirproj);
    dirproj = normalize(dirproj);

    // invert vector for box intersection
    dirproj = 1.f.xxx / vec3(precIssue(dirproj.x), precIssue(dirproj.y), precIssue(dirproj.z));

    // limitation of distance
    bvec3_ bsgn = (bvec3_(sign(dirproj)*ftype_(1.0001f))+true_)>>true_;

    // initial state
    traverseState.defTriangleID = -1;
    traverseState.distMult = dirlen;
    traverseState.diffOffset = 0.f;
    traverseState.idx = SSC(valid) ? 0 : -1;
#ifdef USE_STACKLESS_BVH
    traverseState.bitStack = 0ul;
#endif

    geometrySpace.lastIntersection = eht >= 0 ? hits[eht].uvt : vec4(0.f.xx, INFINITY, FINT_ZERO);
    //geometrySpace.dir = vec4(direct, 1.f);
    
    // calculate longest axis
    geometrySpace.axis = 2;
    {
        vec3 drs = abs(direct); 
        if (drs.y >= drs.x && drs.y > drs.z) geometrySpace.axis = 1;
        if (drs.x >= drs.z && drs.x > drs.y) geometrySpace.axis = 0;
        if (drs.z >= drs.y && drs.z > drs.x) geometrySpace.axis = 2;
    }

    // calculate affine matrices
    vec4 vm = vec4(-direct, 1.f) / (geometrySpace.axis == 0 ? direct.x : (geometrySpace.axis == 1 ? direct.y : direct.z));
    geometrySpace.iM = transpose(mat3(
        geometrySpace.axis == 0 ? vm.wyz : vec3(1.f,0.f,0.f),
        geometrySpace.axis == 1 ? vm.xwz : vec3(0.f,1.f,0.f),
        geometrySpace.axis == 2 ? vm.xyw : vec3(0.f,0.f,1.f)
    ));
    

    // test intersection with main box
    float near = -INFINITY, far = INFINITY;
    const vec2 bndsf2 = vec2(-(1.f+1e-5f), (1.f+1e-5f));
    IF (not(intersectCubeF32Single(torig*dirproj, dirproj, bsgn, mat3x2(bndsf2, bndsf2, bndsf2), near, far))) { 
        traverseState.idx = -1;
    }

    float toffset = max(near, 0.f);
    traverseState.diffOffset = toffset;

    bvhSpace.directInv.xyz = fvec3_(dirproj);
    bvhSpace.minusOrig.xyz = fma(fvec3_(torig), fvec3_(dirproj), -fvec3_(toffset).xxx);
    bvhSpace.boxSide.xyz = bsgn;
    bvhSpace.cutOut = geometrySpace.lastIntersection.z * traverseState.distMult - traverseState.diffOffset; 
    
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
                childIntersect &= intersectCubeDual(bvhSpace.minusOrig.xyz, bvhSpace.directInv.xyz, bvhSpace.boxSide.xyz, 
#ifdef USE_F32_BVH
                    fmat3x4_(bvhBoxes[_cmp][0], bvhBoxes[_cmp][1], bvhBoxes[_cmp][2]),
                    fmat3x4_(vec4(0.f), vec4(0.f), vec4(0.f))
#else
                    fmat3x4_(UNPACK_HF(bvhBoxes[_cmp][0].xy), UNPACK_HF(bvhBoxes[_cmp][1].xy), UNPACK_HF(bvhBoxes[_cmp][2].xy)),
                    fmat3x4_(vec4(0.f), vec4(0.f), vec4(0.f))
#endif
                , nears, fars);
                childIntersect &= bvec2_(lessThanEqual(nears, bvhSpace.cutOut.xx));

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
            if (cnode.x == cnode.y) {
                if (traverseState.defTriangleID < 0) {
                    traverseState.defTriangleID = cnode.x;
                } else {
                    _continue = true;
                }
            }

#ifdef USE_STACKLESS_BVH
            // stackless 
            if (!_continue) {
                // go to parents so far as possible 
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
            if (!_continue) {
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

