
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

const lowp int 
#if defined(ENABLE_VEGA_INSTRUCTION_SET) || defined(ENABLE_TURING_INSTRUCTION_SET) // prefer to run in Turing's too
   localStackSize = 8, pageCount = 4; // 256-bit global memory stack pages
#else
   localStackSize = 4, pageCount = 8; // 128-bit capable 
#endif

#define stack_t int[localStackSize]
#define cacheID ((gl_WorkGroupSize.x*gl_NumWorkGroups.x*WID)+gl_GlobalInvocationID.x)
const highp uint maxIterations = 16384u, GridSize = 64u*2u;
layout ( binding = _CACHE_BINDING, set = 0, std430 ) coherent buffer VT_PAGE_SYSTEM {
    stack_t stack[WORK_SIZE*GridSize];
    stack_t pages[];
} traverseCache;


// stack system of current BVH traverser
//stack_t lstack;
shared stack_t localStack[WORK_SIZE];
#define lstack localStack[Local_Idx]


#ifdef ENABLE_INT16_SUPPORT
#define mint_t int16_t
struct BvhSubState { int idx; int16_t stackPtr, pageID; };
#else
#define mint_t int
struct BvhSubState { int idx, stackPtr, pageID; };
#endif

struct BvhTraverseState {
    int maxElements, entryIDBase, topLevelEntry, defElementID;
    fvec4_ directInv, minusOrig;
} traverseState;

BvhSubState stackState, resrvState;

//BvhSubState[2] stackStates;//stackState, resrvState;
//#define stackState stackStates[0]
//#define resrvState stackStates[1]


#define sidx  stackState.stackPtr


//#define traverseState traverseStates[currentState] // yes, require two states 

// 13.10.2018 added one mandatory stack page, can't be reached by regular operations 
#define CACHE_BLOCK_SIZE ((gl_WorkGroupSize.x*2u)*gl_NumWorkGroups.x*pageCount) // require one reserved block 
#define CACHE_BLOCK (cacheID*pageCount)
#define STATE_PAGE_OFFSET (CACHE_BLOCK_SIZE*currentState)

#define VTX_PTR 0
int cmpt(in int ts){ return clamp(ts,0,localStackSize-1); };

void loadStack(inout int rsl) {
    {[[flatten]] if (stackState.stackPtr <= 0 && stackState.pageID > 0 ) { // make store/load deferred 
        lstack = traverseCache.pages[STATE_PAGE_OFFSET + CACHE_BLOCK + (--stackState.pageID)]; stackState.stackPtr = mint_t(localStackSize);
    };};
    {[[flatten]] if (sidx > 0) { rsl = exchange(lstack[cmpt(--sidx)],-1); };};
};

void storeStack(in int rsl) {
    {[[flatten]] if (stackState.stackPtr >= localStackSize && stackState.pageID < pageCount ) { // make store/load deferred 
        traverseCache.pages[STATE_PAGE_OFFSET + CACHE_BLOCK + (stackState.pageID++)] = lstack; stackState.stackPtr = mint_t(0);
    };};
    {[[flatten]] if (sidx < localStackSize) { lstack[cmpt(sidx++)] = rsl; };};
};


// corrections of box intersection
const bvec4 bsgn = false.xxxx;
const 
float dirlen = 1.f, invlen = 1.f, bsize = 1.f;
