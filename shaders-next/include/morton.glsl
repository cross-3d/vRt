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

#endif
