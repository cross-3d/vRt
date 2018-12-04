
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
   localStackSize = 8, pageCount = 3; // 256-bit global memory stack pages
#else
   localStackSize = 4, pageCount = 7; // 128-bit capable 
#endif

#define stack_t int[localStackSize]
//#define cacheID ((gl_WorkGroupSize.x*gl_NumWorkGroups.x*WID)+gl_GlobalInvocationID.x)
const highp uint maxIterations = 8192u;//* 12u;

//layout ( binding = _CACHE_BINDING, set = 0, align_ssbo ) coherent buffer VT_PAGE_SYSTEM {
//    stack_t stack[WORK_SIZE*GridSize];
//    stack_t pages[];
//} traverseCache;

layout ( binding = _CACHE_BINDING, set = 0, align_ssbo ) subgroupcoherent buffer VT_PAGE_SYSTEM {
    stack_t stack[Wave_Size], pages[Wave_Size*pageCount*2u];
} cache[];


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
    int maxElements, entryIDBase, defElementID;
    fvec4_ directInv, minusOrig;
} traverseState;

BvhSubState stackState, resrvState;
//#define topLevelEntry traverseState.topLevelEntry_

//BvhSubState[2] stackStates;//stackState, resrvState;
//#define stackState stackStates[0]
//#define resrvState stackStates[1]


#define sidx  stackState.stackPtr

//#define CACHE_OFFSET (Wave_Size_RT*currentState+Lane_Idx)*pageCount
//#define CACHE_ID readFLane(gl_WorkGroupID.x*Wave_Count_RT+Wave_Idx) // access should be unified in same subgroups


#define VTX_PTR 0
int cmpt(in int ts){ return clamp(ts,0,localStackSize-1); };

void loadStack(inout int rsl) {
    {[[flatten]] if (stackState.stackPtr <= 0 && stackState.pageID > 0 ) { // make store/load deferred 
        lstack = cache[CACHE_ID].pages[CACHE_OFFSET + (--stackState.pageID)]; stackState.stackPtr = mint_t(localStackSize);
    };};
    {[[flatten]] if (sidx > 0) { rsl = lstack[cmpt(--sidx)]; };};
};

void storeStack(in int rsl) {
    {[[flatten]] if (stackState.stackPtr >= localStackSize && stackState.pageID < pageCount ) { // make store/load deferred 
        cache[CACHE_ID].pages[CACHE_OFFSET + (stackState.pageID++)] = lstack; stackState.stackPtr = mint_t(0);
    };};
    {[[flatten]] if (sidx < localStackSize) { lstack[cmpt(sidx++)] = rsl; };};
};


// corrections of box intersection
const bvec4 bsgn = false.xxxx;
const 
float dirlen = 1.f, invlen = 1.f, bsize = 1.f;
