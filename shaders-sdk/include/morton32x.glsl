// Refreshed morton code library for 8x4 bit data 

#ifdef USE_INT16_FOR_MORTON
    u32x1_t splitBy4(in m8pq u8x1_t a) {
        u16x2_t r = (u16x2_t(a).xx>>u16x2_t(0us,4us))&0xFus;   // ---- ----  ---- 7654  ---- ---- ---- 3210
                r = (r | (r << 6us.xx)) & 0x0303us.xx;          // ---- --76  ---- --54  ---- --32 ---- --10
                r = (r | (r << 3us.xx)) & 0x1111us.xx;          // ---7 ---6  ---5 ---4  ---3 ---2 ---1 ---0
        return u16x2pack(r);
    };
    u32x1_t encodeMorton(in m8pq u8x4_t a) { return u32x1_t((splitBy4(a.x) << 0u) | (splitBy4(a.y) << 1u) | (splitBy4(a.z) << 2u) | (splitBy4(a.w) << 3u)); };
    u32x1_t encodeMorton(in u32x1_t a) { return encodeMorton(u8x4_t(bitfieldExtract(a, 0, 8), bitfieldExtract(a, 8, 8), bitfieldExtract(a, 16, 8), bitfieldExtract(a, 24, 8))); };
#else
    u32x1_t splitBy4(in lowp uint a) {
        u32x1_t r = (a | (a << 12u)) & 0x000F000Fu; // ---- ----  ---- 7654  ---- ---- ---- 3210
                r = (r | (r <<  6u)) & 0x03030303u; // ---- --76  ---- --54  ---- --32 ---- --10
                r = (r | (r <<  3u)) & 0x11111111u; // ---7 ---6  ---5 ---4  ---3 ---2 ---1 ---0
        return r;
    };
    u32x1_t encodeMorton(in lowp uvec4 a) { return u32x1_t((splitBy4(a.x) << 0u) | (splitBy4(a.y) << 1u) | (splitBy4(a.z) << 2u) | (splitBy4(a.w) << 3u)); };
    u32x1_t encodeMorton(in u32x1_t a) { return encodeMorton(uvec4(bitfieldExtract(a, 0, 8), bitfieldExtract(a, 8, 8), bitfieldExtract(a, 16, 8), bitfieldExtract(a, 24, 8))); };
#endif
