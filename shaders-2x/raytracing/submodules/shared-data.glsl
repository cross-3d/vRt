// shared data for traversing, and buffer binding

#define HIT_COUNTER arcounter.hT
#define PREPARED_COUNT arcounter.tT

// for faster BVH traverse
layout ( std430, binding = 0, set = 0 ) buffer unorderedRaysB { ElectedRay unorderedRays[]; };
layout ( std430, binding = 1, set = 0 ) buffer hitsB { HitData hits[]; };

// counters
layout ( std430, binding = 2, set = 0 ) buffer arcounterB { 
    int bT; // blocks counter
    int aT; // active blocks counter
    int pT; // clearing blocks counters
    int tT; // unordered counters
    
    int mT; // available blocks (reusing)
    int rT; // allocator of indice blocks 

    int hT; // hits vertices counters
    int iT; // hits payload counters
} arcounter;

// atomic aggregated counter
initAtomicSubgroupIncFunction(HIT_COUNTER, atomicIncHT, 1, int)
