#ifndef _RANDOM_H
#define _RANDOM_H

uint randomClocks = 1, globalInvocationSMP = 1, subHash = 1;

float floatConstruct( in uint m ) { return fract(1.f + uintBitsToFloat((m & 0x007FFFFFu) | 0x3F800000u)); };
 vec2 floatConstruct( in uvec2 m ) { return vec2(floatConstruct(m.x), floatConstruct(m.y)); };

mediump vec2 halfConstruct ( in uint m ) { return fract(1.f.xx + unpackHalf2x16((m & 0x03FF03FFu) | (0x3C003C00u))); };
highp vec2 unorm16x2Construct( in uint m ) {return clamp01(unpackUnorm2x16(m)); };


// seeds hashers
uint hash ( in uint a ) {
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   return a;
};

// multi-dimensional seeds hashers
uint hash( in uvec2 v ) { return hash(v.y ^ hash(v.x)); };
uint hash( in uvec3 v ) { return hash(v.z ^ hash(v.xy)); };
uint hash( in uvec4 v ) { return hash(v.w ^ hash(v.xyz)); };



// aggregated randoms from seeds
float hrand( in uint   x ) { return floatConstruct(hash(x)); };
float hrand( in uvec2  v ) { return floatConstruct(hash(v)); };
float hrand( in uvec3  v ) { return floatConstruct(hash(v)); };
float hrand( in uvec4  v ) { return floatConstruct(hash(v)); };


// 1D random generators from superseed (quality)
float random( in uvec2 superseed ) {
    const uint hclk = ++randomClocks, comb = hash(uvec3(hclk, subHash, uint(globalInvocationSMP)));
    return floatConstruct(hash(uvec3(comb, superseed)));
};

//  2D random generators from superseed (quality)
vec2 randf2x( in uvec2 superseed ) {
    const uint hclk = ++randomClocks, comb = hash(uvec3(hclk, subHash, uint(globalInvocationSMP)));
    return halfConstruct(hash(uvec3(comb, superseed)));
};

// 2D random generators from superseed 
vec2 randf2q( in uvec2 superseed ) {
    const uint hclk = ++randomClocks, comb = hash(uvec3(hclk, subHash, uint(globalInvocationSMP)));
    return vec2(floatConstruct(hash(uvec2(comb, superseed.x))), floatConstruct(hash(uvec2(comb, superseed.y))));
};

// geometric random generators
vec3 randomSphere(in uvec2 superseed) { return dcts(randf2q(superseed)); };
vec3 randomHemisphereCosine(in uvec2 superseed) {
    const vec2 hmsm = randf2q(superseed);
    const float phi = hmsm.x * TWO_PI, up = sqrt(1.0f - hmsm.y), over = sqrt(fma(up,-up,1.f));
    return vec3(cos(phi)*over,up,sin(phi)*over);
};

#endif
