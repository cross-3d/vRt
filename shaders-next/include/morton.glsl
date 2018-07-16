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

// consist of 4 uint16 as uint32 format
uvec2 encodeMorton(in highp uvec4 a) {
    return splitBy4(a.x) | (splitBy4(a.y) << 1) | (splitBy4(a.z) << 2) | (splitBy4(a.w) << 3);
}

// consist of 4 uint16 as is (packed uvec2)
uvec2 encodeMorton(in uvec2 a) {
#ifdef ENABLE_AMD_INT16
    return encodeMorton(uvec4(unpackUint2x16(a.x), unpackUint2x16(a.y))); // fast packing of RX Vega
#else
    return 
        (splitBy4(bitfieldExtract(a.x, 0 , 16)) << 0) | 
        (splitBy4(bitfieldExtract(a.x, 16, 16)) << 1) | 
        (splitBy4(bitfieldExtract(a.y, 0 , 16)) << 2) | 
        (splitBy4(bitfieldExtract(a.y, 16, 16)) << 3); // fallback method
#endif
}

#endif
