
#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif

const uint currentState = BVH_STATE_TOP;
#define VTX_PTR (currentState == BVH_STATE_TOP ? bvhBlockTop.primitiveOffset : bvhBlockIn.primitiveOffset)

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

// stack system of current BVH traverser
shared int localStack[WORK_SIZE][localStackSize];
#define lstack localStack[Local_Idx]
#define sidx  traverseState.stackPtr
#define STATE_PAGE_OFFSET 0

// BVH traversing state
#define _cacheID gl_GlobalInvocationID.x
struct BvhTraverseState {
         int idx, defElementID, maxElements, entryIDBase, gStackPtr; float diffOffset;
    lowp int stackPtr, pageID;
    fvec4_ directInv, minusOrig;
} traverseState;



void loadStack(inout int rsl) {
    [[flatten]] if (traverseState.stackPtr <= 0 && traverseState.pageID > 0) { // make store/load deferred 
        lstack = pages[_cacheID*pageCount + (--traverseState.pageID) + STATE_PAGE_OFFSET]; traverseState.stackPtr = localStackSize;
    };
    [[flatten]] if ((--sidx) >= 0) rsl = lstack[sidx];
    traverseState.stackPtr = clamp(traverseState.stackPtr, 0, localStackSize);
};

void storeStack(in int rsl) {
    [[flatten]] if (sidx < localStackSize) { const int pti = sidx++; pages[_cacheID*pageCount + (traverseState.pageID) + STATE_PAGE_OFFSET][pti] = (lstack[pti] = rsl); };
    [[flatten]] if (traverseState.stackPtr >= localStackSize && traverseState.pageID < pageCount) { // make store/load deferred 
        pages[_cacheID*pageCount + (traverseState.pageID++) + STATE_PAGE_OFFSET] = lstack; traverseState.stackPtr = 0;
    };
    traverseState.stackPtr = clamp(traverseState.stackPtr, 0, localStackSize);
};



// corrections of box intersection
const bvec4 bsgn = false.xxxx;
const 
float dirlen = 1.f, invlen = 1.f, bsize = 1.f;
