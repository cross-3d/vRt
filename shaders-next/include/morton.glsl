#ifndef _MORTON_H
#define _MORTON_H

uvec2 splitBy4(in highp uint a){
    //uvec2 r = 0u.xx;
    //[[unroll]]
    //for (int i=0;i<8;i++) {
    //    r |= uvec2(bitfieldExtract(a, i, 1) << (i<<2), bitfieldExtract(a, i+8, 1) << (i<<2));
    //}
    //return r;

    uvec2 r = uvec2(bitfieldExtract(a, 0, 8), bitfieldExtract(a, 8, 8)); // ---- ----  ---- ----  ---- ---- fedc ba98   ---- ----  ---- ----  ---- ---- 7654 3210
    r = (r | (r << 12u.xx)) & 0x000F000Fu.xx;                            // ---- ----  ---- fedc  ---- ---- ---- ba98   ---- ----  ---- 7654  ---- ---- ---- 3210
    r = (r | (r <<  6u.xx)) & 0x03030303u.xx;                            // ---- --fe  ---- --dc  ---- --ba ---- --98   ---- --76  ---- --54  ---- --32 ---- --10
    r = (r | (r <<  3u.xx)) & 0x11111111u.xx;                            // ---f ---e  ---d ---c  ---b ---a ---9 ---8   ---7 ---6  ---5 ---4  ---3 ---2 ---1 ---0
    return r;
}

// consist of 4 uint16 as uint32 format
uvec2 encodeMorton(in highp uvec4 a) {
    return 
        (splitBy4(a.x) << 0u) | 
        (splitBy4(a.y) << 1u) | 
        (splitBy4(a.z) << 2u) | 
        (splitBy4(a.w) << 3u);
}

// consist of 4 uint16 as is (packed uvec2)
uvec2 encodeMorton(in uvec2 a) {
#ifdef ENABLE_AMD_INT16
    return encodeMorton(uvec4(unpackUint2x16(a.x), unpackUint2x16(a.y))); // fast packing of RX Vega
#else
    return encodeMorton(uvec4(bitfieldExtract(a.x, 0, 16), bitfieldExtract(a.x, 16, 16), bitfieldExtract(a.y, 0, 16), bitfieldExtract(a.y, 16, 16))); // fallback method
#endif
}

#endif
