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

#ifdef UNIVERSAL_PLATFORM
#define Wave_Size_RT (gl_SubgroupSize.x)
#else
#define Wave_Size_RT (Wave_Size)
#endif

#ifndef OUR_INVOC_TERM
    #define Local_Idx (gl_LocalInvocationID.x)
    #ifdef UNIVERSAL_PLATFORM
        #define Wave_Idx (gl_SubgroupID.x)
        #define Lane_Idx (gl_SubgroupInvocationID.x)
    #else
        #define Wave_Idx (Local_Idx/Wave_Size_RT)
        #define Lane_Idx (Local_Idx%Wave_Size_RT)
    #endif
#endif

#define uvec_wave_ballot uvec4
#define RL_ subgroupBroadcast
#define RLF_ subgroupBroadcastFirst

// universal aliases
#define readFLane RLF_
#define readLane RL_


uvec_wave_ballot ballotHW(in bool i) { return subgroupBallot(i); }
uvec_wave_ballot ballotHW() { return subgroupBallot(true); }
bool electedInvoc() { return subgroupElect(); }


// statically multiplied
#define initAtomicSubgroupIncFunction(mem, fname, by, T)\
T fname() {\
    const uvec_wave_ballot bits = ballotHW();\
    const uint sumInOrder = subgroupBallotBitCount(bits), idxInOrder = subgroupBallotExclusiveBitCount(bits);\
    T gadd = 0;\
    if (subgroupElect() && sumInOrder > 0) {gadd = atomicAdd(mem, T(sumInOrder) * T(by));}\
    return readFLane(gadd) + T(idxInOrder) * T(by);\
}

#define initAtomicSubgroupIncFunctionDyn(mem, fname, T)\
T fname(const T by) {\
    const uvec_wave_ballot bits = ballotHW();\
    const uint sumInOrder = subgroupBallotBitCount(bits), idxInOrder = subgroupBallotExclusiveBitCount(bits);\
    T gadd = 0;\
    if (subgroupElect() && sumInOrder > 0) {gadd = atomicAdd(mem, T(sumInOrder) * T(by));}\
    return readFLane(gadd) + T(idxInOrder) * T(by);\
}


// statically multiplied
#define initAtomicSubgroupIncFunctionTarget(mem, fname, by, T)\
T fname(const uint WHERE) {\
    const uvec_wave_ballot bits = ballotHW();\
    const uint sumInOrder = subgroupBallotBitCount(bits), idxInOrder = subgroupBallotExclusiveBitCount(bits);\
    T gadd = 0;\
    if (subgroupElect() && sumInOrder > 0) {gadd = atomicAdd(mem, T(sumInOrder) * T(by));}\
    return readFLane(gadd) + T(idxInOrder) * T(by);\
}

#define initAtomicSubgroupIncFunctionByTarget(mem, fname, T)\
T fname(const uint WHERE, const T by) {\
    const uvec_wave_ballot bits = ballotHW();\
    const uint sumInOrder = subgroupBallotBitCount(bits), idxInOrder = subgroupBallotExclusiveBitCount(bits);\
    T gadd = 0;\
    if (subgroupElect() && sumInOrder > 0) {gadd = atomicAdd(mem, T(sumInOrder) * T(by));}\
    return readFLane(gadd) + T(idxInOrder) * T(by);\
}


// statically multiplied
#define initSubgroupIncFunctionTarget(mem, fname, by, T)\
T fname(const uint WHERE) {\
    const uvec_wave_ballot bits = ballotHW();\
    const uint sumInOrder = subgroupBallotBitCount(bits), idxInOrder = subgroupBallotExclusiveBitCount(bits);\
    T gadd = 0;\
    if (subgroupElect() && sumInOrder > 0) {gadd = add(mem, T(sumInOrder) * T(by));}\
    return readFLane(gadd) + T(idxInOrder) * T(by);\
}

#define initSubgroupIncFunctionByTarget(mem, fname, T)\
T fname(const uint WHERE, const T by) {\
    const uvec_wave_ballot bits = ballotHW();\
    const uint sumInOrder = subgroupBallotBitCount(bits), idxInOrder = subgroupBallotExclusiveBitCount(bits);\
    T gadd = 0;\
    if (subgroupElect() && sumInOrder > 0) {gadd = add(mem, T(sumInOrder) * T(by));}\
    return readFLane(gadd) + T(idxInOrder) * T(by);\
}



// statically multiplied
#define initSubgroupIncFunctionTargetDual(mem, fname, by, T, T2)\
T2 fname(const uint WHERE, in bvec2 a) {\
    const uvec_wave_ballot bitsx = ballotHW(a.x), bitsy = ballotHW(a.y);\
    const uvec2 \
        sumInOrder = uvec2(subgroupBallotBitCount(bitsx), subgroupBallotBitCount(bitsy)),\
        idxInOrder = uvec2(subgroupBallotExclusiveBitCount(bitsx), subgroupBallotExclusiveBitCount(bitsy));\
    T gadd = 0;\
    if (subgroupElect() && any(greaterThan(sumInOrder, (0u).xx))) {gadd = add(mem, T(sumInOrder.x+sumInOrder.y)*T(by));}\
    return readFLane(gadd).xx + T2(idxInOrder.x, sumInOrder.x+idxInOrder.y) * T(by);\
}



// invoc vote
bool allInvoc(in bool bc){ return subgroupAll(bc); }
bool anyInvoc(in bool bc){ return subgroupAny(bc); }

// aliases
bool allInvoc(in bool_ bc){ return allInvoc(SSC(bc)); }
bool anyInvoc(in bool_ bc){ return anyInvoc(SSC(bc)); }

#define IFALL(b)if(allInvoc(b))
#define IFANY(b)if(anyInvoc(b))


// subgroup barriers
#define SB_BARRIER subgroupMemoryBarrier(),subgroupBarrier();


#endif

