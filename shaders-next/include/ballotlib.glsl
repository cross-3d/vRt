#ifndef _BALLOTLIB_H
#define _BALLOTLIB_H

#include "../include/mathlib.glsl"

// for constant maners
#ifndef Wave_Size
    #ifdef AMD_PLATFORM
        #define Wave_Size 64u
    #else
        #define Wave_Size 32u
    #endif
#endif

//#ifdef UNIVERSAL_PLATFORM
#define Wave_Size_RT (gl_SubgroupSize.x)
//#else
//#define Wave_Size_RT (Wave_Size)
//#endif

#define Wave_Count_RT (gl_NumSubgroups.x)

#ifndef OUR_INVOC_TERM
    #define Global_Idx gl_GlobalInvocationID
    #define Local_Idx (gl_LocalInvocationIndex)
    #define Local_2D (gl_LocalInvocationIndex.xy)
    //#define Global_Idx ()
    //#ifdef UNIVERSAL_PLATFORM
        #define Wave_Idx (gl_SubgroupID.x)
        #define Lane_Idx (gl_SubgroupInvocationID.x)
    //#else
    //    #define Wave_Idx (Local_Idx/Wave_Size_RT)
    //    #define Lane_Idx (Local_Idx%Wave_Size_RT)
    //#endif
#endif

#define uint_ballot uvec4
#define RL_ subgroupBroadcast
#define RLF_ subgroupBroadcastFirst
#define electedInvoc subgroupElect

// universal aliases
#define readFLane RLF_
#define readLane RL_

//uint_ballot ballotHW(in bool i) { return subgroupBallot(i); }
//uint_ballot ballotHW() { return subgroupBallot(true); }
//bool electedInvoc() { return subgroupElect(); }
mediump uvec2 bPrefixSum(in bool val) { const uvec4 blt = subgroupBallot(val); return uvec2(subgroupBallotBitCount(blt), subgroupBallotExclusiveBitCount(blt)); } 
mediump uvec2 bPrefixSum() { const uvec4 blt = subgroupBallot(true); return uvec2(subgroupBallotBitCount(blt), subgroupBallotExclusiveBitCount(blt)); } 

// statically multiplied
#define initAtomicSubgroupIncFunction(mem, fname, by, T)\
T fname() {\
    const mediump uvec2 pfx = bPrefixSum();\
    T gadd = 0;\
    if (subgroupElect()) {gadd = atomicAdd(mem, T(pfx.x) * T(by));}\
    return T(pfx.y) * T(by) + readFLane(gadd);\
}

// statically multiplied
#define initAtomicSubgroupIncFunctionTarget(mem, fname, by, T)\
T fname(in uint WHERE) {\
    const mediump uvec2 pfx = bPrefixSum();\
    T gadd = 0;\
    if (subgroupElect()) {gadd = atomicAdd(mem, T(pfx.x) * T(by));}\
    return T(pfx.y) * T(by) + readFLane(gadd);\
}

// statically multiplied
#define initSubgroupIncFunctionTargetDual(mem, fname, by, T, T2)\
T2 fname(in uint WHERE, in bvec2 a) {\
    const mediump uvec4 pfx2 = uvec4(bPrefixSum(a.x), bPrefixSum(a.y));\
    T gadd = 0;\
    if (subgroupElect() && any(greaterThan(pfx2.xz, (0u).xx))) {gadd = add(mem, T(pfx2.x+pfx2.z)*T(by));}\
    return T(by).xx * T2(pfx2.y, pfx2.w+pfx2.x) + readFLane(gadd).xx;\
}



// invoc vote
bool allInvoc(in bool bc) { return subgroupAll(bc); }
bool anyInvoc(in bool bc) { return subgroupAny(bc); }

// aliases
bool allInvoc(in lowp bool_ bc) { return allInvoc(SSC(bc)); }
bool anyInvoc(in lowp bool_ bc) { return anyInvoc(SSC(bc)); }

#define IFALL(b)if(allInvoc(b))
#define IFANY(b)if(anyInvoc(b))


// subgroup barriers
#define SB_BARRIER subgroupMemoryBarrier(),subgroupBarrier();
#define LGROUP_BARRIER groupMemoryBarrier(),barrier();

#endif

