#ifndef _BALLOTLIB_H
#define _BALLOTLIB_H

#include "../include/mathlib.glsl"

// alternate of https://devblogs.nvidia.com/cuda-pro-tip-optimized-filtering-warp-aggregated-atomics/


// for constant maners
#ifndef Wave_Size
    #if (defined(AMD_PLATFORM) || defined(ENABLE_TURING_INSTRUCTION_SET))
        #define Wave_Size 64u
    #else
        #define Wave_Size 32u
    #endif
#endif

//#ifdef IS_RAY_GEN
#define Ni 2u
//#else
//#define Ni 1u
//#endif

#ifdef IS_RAY_SHADER
//#define CPC 2304u
#define WID (0u+gl_LaunchIDNV[Ni])
#else
#define WID (0u+gl_GlobalInvocationID[Ni])
#endif

//#ifdef UNIVERSAL_PLATFORM
#define Wave_Size_RT (gl_SubgroupSize.x)
//#else
//#define Wave_Size_RT (Wave_Size)
//#endif

#define Wave_Count_RT (gl_NumSubgroups.x)

#ifndef OUR_INVOC_TERM
    #define Launch_Idx (gl_GlobalInvocationID.xy)
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

// subgroup barriers
#define LGROUP_BARRIER memoryBarrier(),subgroupBarrier();
#define IFALL(b) [[flatten]]if(subgroupAll(b))
#define IFANY(b)            if(subgroupAny(b))

//uint_ballot ballotHW(in bool i) { return subgroupBallot(i); }
//uint_ballot ballotHW() { return subgroupBallot(true); }
//bool electedInvoc() { return subgroupElect(); }

//lowp uvec2 bPrefixSum(in bool val) { const uvec4 blt = subgroupBallot(val); return uvec2(subgroupBallotBitCount(blt), subgroupBallotExclusiveBitCount(blt)); };
//lowp uvec2 bPrefixSum() { const uvec4 blt = subgroupBallot(true); return uvec2(subgroupBallotBitCount(blt), subgroupBallotExclusiveBitCount(blt)); }; 

const uint UONE = 1u;
//lowp uvec2 bPrefixSum(in bool val) { return uvec2(subgroupAdd(uint(val)), subgroupExclusiveAdd(uint(val))); };
lowp uvec2 bPrefixSum() { return uvec2(subgroupAdd(UONE), subgroupExclusiveAdd(UONE)); };

/*
// advanced version
void bPrefixSum(in bvec4 val, inout lowp uvec4 sums, inout lowp uvec4 pfxs) { 
    {uvec4 blt = subgroupBallot(val.x); sums.x = subgroupBallotBitCount(blt), pfxs.x = subgroupBallotExclusiveBitCount(blt);};
    {uvec4 blt = subgroupBallot(val.y); sums.y = subgroupBallotBitCount(blt), pfxs.y = subgroupBallotExclusiveBitCount(blt);};
    {uvec4 blt = subgroupBallot(val.z); sums.z = subgroupBallotBitCount(blt), pfxs.z = subgroupBallotExclusiveBitCount(blt);};
    {uvec4 blt = subgroupBallot(val.w); sums.w = subgroupBallotBitCount(blt), pfxs.w = subgroupBallotExclusiveBitCount(blt);};
};
*/


// statically multiplied
#define initAtomicSubgroupIncFunction(mem, fname, by, T)\
T fname() {\
    const lowp uvec2 pfx = bPrefixSum();\
    T gadd = 0; [[flatten]] if (subgroupElect()) {gadd = atomicAdd(mem, T(pfx.x) * T(by));};\
    return T(pfx.y) * T(by) + readFLane(gadd);\
};
/*
#ifdef REGULAR_ATOMIC_INC
#define initAtomicSubgroupIncFunctionTarget(mem, fname, by, T)\
T fname(in uint WHERE) {\
    return atomicAdd(mem, T(1u));\
}
#else*/
// statically multiplied
#define initAtomicSubgroupIncFunctionTarget(mem, fname, by, T)\
T fname(in  uint WHERE) {\
    const lowp uvec2 pfx = bPrefixSum();\
    T gadd = 0; [[flatten]] if (subgroupElect()) {gadd = atomicAdd(mem, T(pfx.x) * T(by));};\
    return T(pfx.y) * T(by) + readFLane(gadd);\
};

#define initAtomicSubgroupIncFunctionTargetBinarity(mem, fname, by, T)\
T fname(in  uint WHERE) {\
    const lowp uvec2 pfx = bPrefixSum();\
    T gadd = 0; [[flatten]] if (subgroupElect()) {gadd = atomicAdd(mem[WID], T(pfx.x) * T(by));};\
    return T(pfx.y) * T(by) + readFLane(gadd);\
};

//#endif
//

// statically multiplied
#define initSubgroupIncFunctionTarget(mem, fname, by, T)\
T fname(in  uint WHERE) {\
    const lowp uvec2 pfx = bPrefixSum();\
    T gadd = 0; [[flatten]] if (subgroupElect()) {gadd = add(mem, T(pfx.x) * T(by));};\
    return T(pfx.y) * T(by) + readFLane(gadd);\
};

/*
// statically multiplied
#define initSubgroupIncFunctionTargetDual(mem, fname, by, T, T2)\
T2 fname(in uint WHERE, in bvec2 a) {\
    const lowp uvec4 pfx2 = uvec4(bPrefixSum(a.x), bPrefixSum(a.y));\
    T gadd = 0;\
    if (subgroupElect() && any(greaterThan(pfx2.xz, (0u).xx))) {gadd = add(mem, T(pfx2.x+pfx2.z)*T(by));}\
    return T(by).xx * T2(pfx2.y, pfx2.w+pfx2.x) + readFLane(gadd).xx;\
}

const lowp uvec4 pfx0[2] = { 0u.xxxx, 0u.xxxx };
//lowp uvec4 pfx[2] = pfx0; bPrefixSum(a, pfx[0], pfx[1]);
// lowp umat2x4 pfx = transpose(umat4x2(bPrefixSum(a.x), bPrefixSum(a.y), bPrefixSum(a.z), bPrefixSum(a.w)));

// statically multiplied
#define initSubgroupIncFunctionTargetQuad(mem, fname, by, T, T4)\
T4 fname(in uint WHERE, in bvec4 a) {\
    lowp uvec4 pfx[2] = pfx0; bPrefixSum(a, pfx[0], pfx[1]);\
    T gadd = 0;\
    if (subgroupElect() && any(greaterThan(pfx[0], (0u).xxxx))) {gadd = add(mem, T(pfx[0].x+pfx[0].y+pfx[0].z+pfx[0].w)*T(by));}\
    return T(by).xxxx * (T4(pfx[1]) + T4(0u, pfx[0].xyz) + T4(0u.xx, pfx[0].xy) + T4(0u.xxx, pfx[0].x)) + readFLane(gadd).xxxx;\
}
*/



// invoc vote
//bool allInvoc(in bool bc) { return subgroupAll(bc); }
//bool anyInvoc(in bool bc) { return subgroupAny(bc); }

// aliases
//bool allInvoc(in pbool_ bc) { return allInvoc(SSC(bc)); }
//bool anyInvoc(in pbool_ bc) { return anyInvoc(SSC(bc)); }

#endif
