
#ifndef _CACHE_BINDING
#define _CACHE_BINDING 9
#endif

#ifndef _RAY_TYPE
#define _RAY_TYPE VtRay
#endif


#ifndef fpInner
//const float fpInner = 0.0000152587890625f, fpOne = 1.f;
#define fpInner 0.0000152587890625f
#define fpOne 1.f
#endif

#if defined(ENABLE_VEGA_INSTRUCTION_SET) || defined(ENABLE_TURING_INSTRUCTION_SET) // prefer to run in Turing's too
//#ifdef ENABLE_VEGA_INSTRUCTION_SET
  const  lowp  int localStackSize = 8, pageCount = 4; // 256-bit global memory stack pages
#else
  const  lowp  int localStackSize = 4, pageCount = 8; // 128-bit capable 
#endif
  //const highp uint maxIterations  = 8192u;
  const highp uint maxIterations  = 16384u;


const uint GridSize = 72u*2u; // specialization for future implementation 
layout ( binding = _CACHE_BINDING, set = 0, std430 ) coherent buffer VT_PAGE_SYSTEM {
    int stack[WORK_SIZE*GridSize][localStackSize];
    int pages[][localStackSize];
} traverseCache;


// stack system of current BVH traverser
shared int localStack[WORK_SIZE][localStackSize];
uint currentState = BVH_STATE_TOP;
#define lstack localStack[Local_Idx]
#define sidx  traverseState.stackPtr

// BVH traversing state
#define _cacheID ((gl_WorkGroupSize.x*gl_NumWorkGroups.x*WID)+gl_GlobalInvocationID.x)
struct BvhTraverseState {
    int maxElements, entryIDBase, diffOffset, defElementID; bool saved;
    int idx;    lowp int stackPtr   , pageID;
    int idxTop; lowp int stackPtrTop, pageIDTop;
    fvec4_ directInv, minusOrig;
} traverseState;
//#define traverseState traverseStates[currentState] // yes, require two states 

// 13.10.2018 added one mandatory stack page, can't be reached by regular operations 
#define CACHE_BLOCK_SIZE ((gl_WorkGroupSize.x*2u)*gl_NumWorkGroups.x*pageCount) // require one reserved block 
#define CACHE_BLOCK (_cacheID*pageCount)
#define STATE_PAGE_OFFSET (CACHE_BLOCK_SIZE*currentState)

//#define VTX_PTR (currentState == BVH_STATE_TOP ? bvhBlockTop.primitiveOffset : bvhBlockIn.primitiveOffset)
#define VTX_PTR 0
int cmpt(in int ts){ return clamp(ts,0,localStackSize-1); };

void loadStack(inout int rsl) {
    [[flatten]] if (traverseState.stackPtr <= 0 && traverseState.pageID > 0 ) { // make store/load deferred 
        lstack = traverseCache.pages[STATE_PAGE_OFFSET + CACHE_BLOCK + (--traverseState.pageID)]; traverseState.stackPtr = localStackSize;
    };
    [[flatten]] if (sidx > 0) { rsl = exchange(lstack[cmpt(--sidx)],-1); };
};

void storeStack(in int rsl) {
    [[flatten]] if (traverseState.stackPtr >= localStackSize && traverseState.pageID < pageCount ) { // make store/load deferred 
        traverseCache.pages[STATE_PAGE_OFFSET + CACHE_BLOCK + (traverseState.pageID++)] = lstack; traverseState.stackPtr = 0;
    };
    [[flatten]] if (sidx < localStackSize) { lstack[cmpt(sidx++)] = rsl; };
};


// corrections of box intersection
const bvec4 bsgn = false.xxxx;
const 
float dirlen = 1.f, invlen = 1.f, bsize = 1.f;
