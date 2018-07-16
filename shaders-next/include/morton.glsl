#ifndef _MORTON_H
#define _MORTON_H


uvec2 splitBy4(in highp uint a){
    uvec2 r = 0u.xx;
    [[unroll]]
    for (int i=0;i<8;i++) {
        r |= uvec2(bitfieldExtract(a, i, 1) << (i*4), bitfieldExtract(a, i+8, 1) << (i*4));
    }
    return r;
}

uvec2 encodeMorton3_64(in highp uvec4 a) {
    return splitBy4(a.x) | (splitBy4(a.y) << 1) | (splitBy4(a.z) << 2) | (splitBy4(a.w) << 3);
}


#if defined(INTEL_PLATFORM)  // most devices does not support 64-bit directly

//uint splitBy3(in uint a){
//    uint x = a & 0x3ffu;
//    x = (x | x << 16u) & 0x30000ffu;
//    x = (x | x << 8u) & 0x300f00fu;
//    x = (x | x << 4u) & 0x30c30c3u;
//    x = (x | x << 2u) & 0x9249249u;
//    return x;
//}

//uvec2 encodeMorton3_64(in uvec3 a) {
//    return uvec2(0u, splitBy3(a.x) | (splitBy3(a.y) << 1) | (splitBy3(a.z) << 2));
//}

#else

// method to seperate bits from a given integer 3 positions apart
//uint64_t splitBy3(in uint a){
//    uint64_t x = P2U(uvec2(a & 0x1fffffu, 0u));
//    x = (x | (x << 32ul)) & 0x1f00000000fffful;
//    x = (x | (x << 16ul)) & 0x1f0000ff0000fful;
//    x = (x | (x << 8ul)) & 0x100f00f00f00f00ful;
///    x = (x | (x << 4ul)) & 0x10c30c30c30c30c3ul;
//    x = (x | (x << 2ul)) & 0x1249249249249249ul;
//    return x;
//}

//uvec2 encodeMorton3_64(in uvec3 a) {
//    return U2P(splitBy3(a.x) | (splitBy3(a.y) << 1) | (splitBy3(a.z) << 2));
//}

#endif

#endif
