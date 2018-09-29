#ifdef USE_INT16_FOR_MORTON
    // minimal supported
    #define U8s uint16_t
    #define U8v4 u16vec4

    // have no direct support...
    //#define U8s uint8_t
    //#define U8v4 u8vec4

    // use uint16_t if possible
    uint splitBy4(in U8s a) {
        u16vec2 r = u16vec2(a&U8s(0xFus),a>>U8s(4us)); // ---- ----  ---- 7654  ---- ---- ---- 3210
                r = (r | (r << 6us.xx)) & 0x0303us.xx; // ---- --76  ---- --54  ---- --32 ---- --10
                r = (r | (r << 3us.xx)) & 0x1111us.xx; // ---7 ---6  ---5 ---4  ---3 ---2 ---1 ---0
        return packUint2x16(r);
    }

    // consist of 4 uint8 as uint32 format
    uint encodeMorton(in U8v4 a) {
        return 
            (splitBy4(a.x) << 0us) | 
            (splitBy4(a.y) << 1us) | 
            (splitBy4(a.z) << 2us) | 
            (splitBy4(a.w) << 3us);
    }
#else
    // fallback support
    #define U8s uint
    #define U8v4 uvec4

    uint splitBy4(in lowp uint a) {
        //uint r = uint(a&0xFFu);
        uint r = a;
             r = (r | (r << 12u)) & 0x000F000Fu; // ---- ----  ---- 7654  ---- ---- ---- 3210
             r = (r | (r <<  6u)) & 0x03030303u; // ---- --76  ---- --54  ---- --32 ---- --10
             r = (r | (r <<  3u)) & 0x11111111u; // ---7 ---6  ---5 ---4  ---3 ---2 ---1 ---0
        return r;
    }

    // consist of 4 uint8 as uint32 format
    uint encodeMorton(in lowp uvec4 a) {
        return 
            (splitBy4(a.x) << 0u) | 
            (splitBy4(a.y) << 1u) | 
            (splitBy4(a.z) << 2u) | 
            (splitBy4(a.w) << 3u);
    }
#endif

// consist of 4 uint8
uint encodeMorton(in uint a) {
    return encodeMorton(U8v4(bitfieldExtract(a, 0, 8), bitfieldExtract(a, 8, 8), bitfieldExtract(a, 16, 8), bitfieldExtract(a, 24, 8))); // fallback method
}
