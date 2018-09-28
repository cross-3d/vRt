#ifdef ENABLE_INT16_SUPPORT
    // minimal supported
    #define U8s uint16_t
    #define U8v4 u16vec4

    // no direct support by AMD GPU's
    //#define U8s uint8_t
    //#define U8v4 u8vec4

    // 
    uint splitBy4_low(in U8s a) {
        u16vec2 r = u16vec2(a&U8s(0xFus),a>>U8s(4us)); // ---- ----  ---- 7654  ---- ---- ---- 3210
                r = (r | (r << 6us.xx)) & 0x0303us.xx; // ---- --76  ---- --54  ---- --32 ---- --10
                r = (r | (r << 3us.xx)) & 0x1111us.xx; // ---7 ---6  ---5 ---4  ---3 ---2 ---1 ---0
        return packUint2x16(r);
    }

    // use dual uvec2
    uvec2 splitBy4(in uint16_t a) {
        uvec2 r = uvec2(a&0xFFus,a>>8us);           // ---- ----  ---- ----  ---- ---- fedc ba98   ---- ----  ---- ----  ---- ---- 7654 3210
        r = (r | (r << 12u.xx)) & 0x000F000Fu.xx;   // ---- ----  ---- fedc  ---- ---- ---- ba98   ---- ----  ---- 7654  ---- ---- ---- 3210
        r = (r | (r <<  6u.xx)) & 0x03030303u.xx;   // ---- --fe  ---- --dc  ---- --ba ---- --98   ---- --76  ---- --54  ---- --32 ---- --10
        r = (r | (r <<  3u.xx)) & 0x11111111u.xx;   // ---f ---e  ---d ---c  ---b ---a ---9 ---8   ---7 ---6  ---5 ---4  ---3 ---2 ---1 ---0
        return r;
        //return splitBy4_low(U8s(a&0xFFus),U8s(a>>8us));
    }

    // consist of 4 uint16 as uint32 format
    uvec2 encodeMorton(in u16vec4 a) {
        return 
            (splitBy4(a.x) << 0u) | 
            (splitBy4(a.y) << 1u) | 
            (splitBy4(a.z) << 2u) | 
            (splitBy4(a.w) << 3u);
    }
#else
    // use dual uvec2
    uvec2 splitBy4(in highp uint a) {
        uvec2 r = uvec2(a&0xFFu,a>>8u);             // ---- ----  ---- ----  ---- ---- fedc ba98   ---- ----  ---- ----  ---- ---- 7654 3210
        r = (r | (r << 12u.xx)) & 0x000F000Fu.xx;   // ---- ----  ---- fedc  ---- ---- ---- ba98   ---- ----  ---- 7654  ---- ---- ---- 3210
        r = (r | (r <<  6u.xx)) & 0x03030303u.xx;   // ---- --fe  ---- --dc  ---- --ba ---- --98   ---- --76  ---- --54  ---- --32 ---- --10
        r = (r | (r <<  3u.xx)) & 0x11111111u.xx;   // ---f ---e  ---d ---c  ---b ---a ---9 ---8   ---7 ---6  ---5 ---4  ---3 ---2 ---1 ---0
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
#endif

// consist of 4 uint16 as is (packed uvec2)
uvec2 encodeMorton(in uvec2 a) {
#ifdef ENABLE_INT16_SUPPORT
    return encodeMorton(u16vec4(unpackUint2x16(a.x), unpackUint2x16(a.y))); // fast packing of RX Vega
#else
    return encodeMorton(uvec4(bitfieldExtract(a.x, 0, 16), bitfieldExtract(a.x, 16, 16), bitfieldExtract(a.y, 0, 16), bitfieldExtract(a.y, 16, 16))); // fallback method
#endif
}
