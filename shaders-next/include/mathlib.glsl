#ifndef _MATHLIB_H
#define _MATHLIB_H

// here also support of true half float and int16...



// NEXT standard consts in current
// Ray tracing NEXT capable shader standard development planned begin in 2019 year
//const float PZERO = 0.0001f;
const float PHI = 1.6180339887498948482f;
const float INFINITY = 1e+4f, IOFF = 1e-4f;
const float PI = 3.1415926535897932384626422832795028841971f;
const float TWO_PI = 6.2831853071795864769252867665590057683943f;
const float SQRT_OF_ONE_THIRD = 0.5773502691896257645091487805019574556476f;
const float E = 2.7182818284590452353602874713526624977572f;


const float N1024 = 1024.f;
#ifdef AMD_F16_BVH
const float16_t InZero = 0.0009765625hf, InOne = 1.0009765625hf;
#else
const float InZero = 0.00000011920928955078125f, InOne = 1.00000011920928955078125f;
#endif


#ifdef AMD_F16_BVH
const float16_t One1024 = 0.0009765625hf;
#else
const float One1024 = 0.0009765625f;
#endif


// float 16 or 32 bit types
#ifdef AMD_F16_BVH
#define ftype_ float16_t
#define fvec3_ f16vec3
#define fvec4_ f16vec4
#define fvec2_ f16vec2
#define fmat2x4_ f16mat2x4
#define fmat4x4_ f16mat4x4
#define fmat3x4_ f16mat3x4
#define fmat3x2_ f16mat3x2
#define fmat4x3_ f16mat4x3
#else 
#define ftype_ float
#define fvec2_ vec2
#define fvec3_ vec3
#define fvec4_ vec4
#define fmat2x4_ mat2x4
#define fmat4x4_ mat4x4
#define fmat3x4_ mat3x4
#define fmat3x2_ mat3x2
#define fmat4x3_ mat4x3
#endif


const float _FZERO = 0.f;
const int _IZERO = 0;

//#ifdef ENABLE_AMD_INSTRUCTION_SET
//    #define ISTORE(img, crd, data) imageStoreLodAMD(img,crd,_IZERO,data)
//    #define SGATHER(smp, crd, chnl) textureGatherLodAMD(smp,crd,_FZERO,chnl)
//#else
    #define ISTORE(img, crd, data) imageStore(img,crd,data)
    #define SGATHER(smp, crd, chnl) textureGather(smp,crd,chnl)
//#endif


// AMD min3/max3 ...
#ifdef ENABLE_AMD_INSTRUCTION_SET
#define min3_wrap(a,b,c) min3(a,b,c)
#define max3_wrap(a,b,c) max3(a,b,c)
#else
#define min3_wrap(a,b,c) min(min(a,b),c)
#define max3_wrap(a,b,c) max(max(a,b),c)
#endif
#define max3_vec(a) max3_wrap(a.x,a.y,a.z)
#define min3_vec(a) min3_wrap(a.x,a.y,a.z)


#ifdef ENABLE_AMD_INSTRUCTION_SET
#define mid3_wrap(a,b,c) mid3(a,b,c)
#else
float mid3_wrap(in float a, in float b, in float c) {
    const float m = max3_wrap(a, b, c);
    [[flatten]] if (m == a) { return max(b, c); } else if (m == b) { return max(a, c); } else { return max(a, b); }
}

vec4 mid3_wrap(in vec4 a, in vec4 b, in vec4 c) {
    return vec4(
        mid3_wrap(a.x,b.x,c.x),
        mid3_wrap(a.y,b.y,c.y),
        mid3_wrap(a.z,b.z,c.z),
        mid3_wrap(a.w,b.w,c.w)
    );
}
#endif


#define bvec4_ uvec4
#define bvec3_ uvec3
#define bvec2_ uvec2
#define bool_ uint

const lowp bool_ true_ = bool_(1u), false_ = bool_(0u); 
const lowp bvec2_ true2_ = true_.xx, false2_ = false_.xx;

// null of indexing in float representation
const uint UINT_ZERO = 0x0u, UINT_NULL = 0xFFFFFFFFu;
#define FINT_NULL uintBitsToFloat(UINT_NULL)
#define FINT_ZERO uintBitsToFloat(UINT_ZERO)

/*
// inprecise comparsion functions
const float PRECERR = PZERO;
lowp bool_ lessEqualF   (in float a, in float b) { return bool_(   (a-b) <=  PRECERR); }
lowp bool_ lessF        (in float a, in float b) { return bool_(   (a-b) <  -PRECERR); }
lowp bool_ greaterEqualF(in float a, in float b) { return bool_(   (a-b) >= -PRECERR); }
lowp bool_ greaterF     (in float a, in float b) { return bool_(   (a-b) >   PRECERR); }
lowp bool_ equalF       (in float a, in float b) { return bool_(abs(a-b) <=  PRECERR); }
*/

// precision utils
//#define precIssue(a) (sign(a)*max(abs(a),1e-5f))
#define precIssue(a) (a)


// vector math utils
float sqlen(in vec3 a) { return dot(a, a); }
float sqlen(in vec2 a) { return dot(a, a); }
float sqlen(in float v) { return v * v; }
int modi(in int a, in int b) { return (a % b + b) % b; }
vec4 divW(in vec4 aw) { return aw / max(abs(aw.w),0.00000011920928955078125f); }
vec3 rotate_vector( in vec4 quat, in vec3 vec ) { return vec + 2.0 * cross( cross( vec, quat.xyz ) + quat.w * vec, quat.xyz ); }
vec4 rotation_quat( in vec3 axis, in float half_angle ) { return vec4(axis * sin(half_angle), cos(half_angle)); }

// memory managment
void swap(inout  int a, inout  int b) {  int t = a; a = b; b = t; }
void swap(inout uint a, inout uint b) { uint t = a; a = b; b = t; }

uint exchange(inout uint mem, in uint v) { uint tmp = mem; mem = v; return tmp; }
 int exchange(inout  int mem, in  int v) {  int tmp = mem; mem = v; return tmp; }

uint add(inout uint mem, in uint ops) { uint tmp = mem; mem += ops; return tmp; }
 int add(inout  int mem, in  int ops) {  int tmp = mem; mem += ops; return tmp; }


// logical functions (bvec2)
bvec2 not(in bvec2 a) { return bvec2(!a.x, !a.y); }
bvec2 and(in bvec2 a, in bvec2 b) { return bvec2(a.x && b.x, a.y && b.y); }
bvec2 or(in bvec2 a, in bvec2 b) { return bvec2(a.x || b.x, a.y || b.y); }

// logical functions (bvec4)
bvec4 or(in bvec4 a, in bvec4 b) { return bvec4(a.x || b.x, a.y || b.y, a.z || b.z, a.w || b.w); }
bvec4 and(in bvec4 a, in bvec4 b) { return bvec4(a.x && b.x, a.y && b.y, a.z && b.z, a.w && b.w); }
bvec4 not(in bvec4 a) { return bvec4(!a.x, !a.y, !a.z, !a.w); }

// mixing functions
void mixed(inout float src, inout float dst, in float coef) { dst *= coef; src *= 1.0f - coef; }
void mixed(inout vec3 src, inout vec3 dst, in float coef) { dst *= coef; src *= 1.0f - coef; }
void mixed(inout vec3 src, inout vec3 dst, in vec3 coef) { dst *= coef; src *= 1.0f - coef; }

float clamp01(in float c) {return clamp(c, 0.0f.x,    0.99999f.x);    };
vec2  clamp01(in vec2 c)  {return clamp(c, 0.0f.xx,   0.99999f.xx);   };
vec3  clamp01(in vec3 c)  {return clamp(c, 0.0f.xxx,  0.99999f.xxx);  };
vec4  clamp01(in vec4 c)  {return clamp(c, 0.0f.xxxx, 0.99999f.xxxx); };

// matrix math (simular DX12)
vec4 mult4(in vec4 vec, in mat4 tmat) { return tmat * vec; }
vec4 mult4(in mat4 tmat, in vec4 vec) { return vec * tmat; }


// 64-bit packing
#define U2P unpackUint2x32
#define P2U packUint2x32

// 128-bit packing (2x64bit)
uvec4 U4P(in u64vec2 pckg) { return uvec4(U2P(pckg.x), U2P(pckg.y)); }
u64vec2 P4U(in uvec4 pckg) { return u64vec2(uint64_t(P2U(pckg.xy)), uint64_t(P2U(pckg.zw))); }

// float packing
uvec2 packHalf4x16(in vec4 floats) { return uvec2(packHalf2x16(floats.xy), packHalf2x16(floats.zw)); }
vec4 unpackHalf4x16(in uvec2 hilo) { return vec4(unpackHalf2x16(hilo.x), unpackHalf2x16(hilo.y)); }


// bit utils
int lsb(in uint vlc) { return findLSB(vlc); }
int msb(in uint vlc) { return findMSB(vlc); }
uint bitcnt(in uint vlc) { return uint(bitCount(vlc)); }
uint bitcnt(in uvec2 lh) { ivec2 bic = bitCount(lh); return uint(bic.x+bic.y); }
uint bitcnt(in uint64_t lh) { ivec2 bic = bitCount(U2P(lh)); return uint(bic.x+bic.y); }


// bit measure utils
int lsb(in uvec2 pair) {
#ifdef AMD_PLATFORM
    return findLSB(P2U(pair));
#else
    const ivec2 hl = findLSB(pair); return (hl.x >= 0) ? hl.x : (32 + hl.y);
#endif
}

int msb(in uvec2 pair) {
#ifdef AMD_PLATFORM
    return findMSB(P2U(pair));
#else
    const ivec2 hl = findMSB(pair); return (hl.y >= 0) ? (32 + hl.y) : hl.x;
#endif
}

int msb(in uint64_t vlc) { 
#ifdef AMD_PLATFORM
    return findMSB(vlc);
#else
    return msb(U2P(vlc));
#endif
}

int lsb(in uint64_t vlc) { 
#ifdef AMD_PLATFORM
    return findLSB(vlc);
#else
    return lsb(U2P(vlc));
#endif
}




// bit insert and extract
 int BFE_HW(in  int base, in  int offset , in int bits) { return bitfieldExtract(base, offset, bits); }
uint BFE_HW(in uint base, in  int offset , in int bits) { return bitfieldExtract(base, offset, bits); }
 int BFI_HW(in  int base, in  int inserts, in int offset, in int bits) { return bitfieldInsert(base, inserts, offset, bits); }
uint BFI_HW(in uint base, in uint inserts, in int offset, in int bits) { return bitfieldInsert(base, inserts, offset, bits); }

// int operations
 int tiled(in  int x, in  int y) {return x/y + int(x%y != 0); }
uint tiled(in uint x, in uint y) {return x/y + int(x%y != 0); }


// precise optimized mix/lerp
#define _FMOP fma(b,c,fma(a,-c,a)) // fma based mix/lerp
float fmix(in float a, in float b, in float c) { return _FMOP; }
vec2 fmix(in vec2 a, in vec2 b, in vec2 c) { return _FMOP; }
vec3 fmix(in vec3 a, in vec3 b, in vec3 c) { return _FMOP; }
vec4 fmix(in vec4 a, in vec4 b, in vec4 c) { return _FMOP; }
#ifdef ENABLE_AMD_INSTRUCTION_SET
float16_t fmix(in float16_t a, in float16_t b, in float16_t c) { return _FMOP; }
f16vec2 fmix(in f16vec2 a, in f16vec2 b, in f16vec2 c) { return _FMOP; }
f16vec3 fmix(in f16vec3 a, in f16vec3 b, in f16vec3 c) { return _FMOP; }
f16vec4 fmix(in f16vec4 a, in f16vec4 b, in f16vec4 c) { return _FMOP; }
#endif





// color space utils
const float HDR_GAMMA = 2.2f;
vec3 fromLinear(in vec3 linearRGB) { return mix(vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055), linearRGB * vec3(12.92), lessThan(linearRGB, vec3(0.0031308))); }
vec3 toLinear(in vec3 sRGB) { return mix(pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4)), sRGB/vec3(12.92), lessThan(sRGB, vec3(0.04045))); }
vec4 fromLinear(in vec4 linearRGB) { return vec4(fromLinear(linearRGB.xyz), linearRGB.w); }
vec4 toLinear(in vec4 sRGB) { return vec4(toLinear(sRGB.xyz), sRGB.w); }

// boolean binary compatibility
bool SSC(in lowp bool_ b) {return bool(b);}
bvec2 SSC(in lowp bvec2_ b) {return bvec2(b);}
bvec4 SSC(in lowp bvec4_ b) {return bvec4(b);}

bool SSC(in bool b) {return b;}
bvec2 SSC(in bvec2 b) {return b;}
bvec4 SSC(in bvec4 b) {return b;}

lowp bool_ any(in lowp bvec2_ b) {return b.x|b.y;}
lowp bool_ all(in lowp bvec2_ b) {return b.x&b.y;}
lowp bool_ not(in lowp bool_ b) {return true_^b;}
lowp bvec2_ not(in lowp bvec2_ b) {return true2_^b;}

#define IF(b)if(SSC(b))




// select by boolean
int mix(in int a, in int b, in lowp bool_ c) { return mix(a,b,SSC(c)); }
uint mix(in uint a, in uint b, in lowp bool_ c) { return mix(a,b,SSC(c)); }
float mix(in float a, in float b, in lowp bool_ c) { return mix(a,b,SSC(c)); }
ivec2 mix(in ivec2 a, in ivec2 b, in lowp bvec2_ c) { return mix(a,b,SSC(c)); }
uvec2 mix(in uvec2 a, in uvec2 b, in lowp bvec2_ c) { return mix(a,b,SSC(c)); }
vec2 mix(in vec2 a, in vec2 b, in lowp bvec2_ c) { return mix(a,b,SSC(c)); }
vec4 mix(in vec4 a, in vec4 b, in lowp bvec4_ c) { return mix(a,b,SSC(c)); }

// 16-bit int/uint
#ifdef ENABLE_AMD_INT16
 int16_t mix(in  int16_t a, in  int16_t b, in lowp bool_ c) { return mix(a,b,SSC(c)); }
uint16_t mix(in uint16_t a, in uint16_t b, in lowp bool_ c) { return mix(a,b,SSC(c)); }
i16vec2 mix(in i16vec2 a, in i16vec2 b, in lowp bvec2_ c) { return mix(a,b,SSC(c)); }
u16vec2 mix(in u16vec2 a, in u16vec2 b, in lowp bvec2_ c) { return mix(a,b,SSC(c)); }
#endif

// 16-bit float
#ifdef ENABLE_AMD_INSTRUCTION_SET
float16_t mix(in float16_t a, in float16_t b, in lowp bool_ c) { return mix(a,b,SSC(c)); }
f16vec2 mix(in f16vec2 a, in f16vec2 b, in lowp bvec2_ c) { return mix(a,b,SSC(c)); }
f16vec4 mix(in f16vec4 a, in f16vec4 b, in lowp bvec4_ c) { return mix(a,b,SSC(c)); }
#endif


// swap of 16-bits by funnel shifts and mapping 
uint fast16swap(in uint b32, const lowp bool_ nswp) {
    const uint vrc = 16u - uint(nswp) * 16u;
    return (b32 << (vrc)) | (b32 >> (32u-vrc));
}

uint64_t fast32swap(in uint64_t b64, const lowp bool_ nswp) {
    const uint64_t vrc = 32ul - uint64_t(nswp) * 32ul;
    return (b64 << (vrc)) | (b64 >> (64ul-vrc));
}


// swap x and y swizzle by funnel shift (AMD half float)
#ifdef ENABLE_AMD_INSTRUCTION_SET
f16vec2 fast16swap(in f16vec2 b32, in lowp bool_ nswp) { 
    return mix(b32.yx, b32, nswp.xx); // use swizzle version (some device can be slower)
}
#endif

// swap x and y swizzle by funnel shift
vec2 fast32swap(in vec2 b64, in lowp bool_ nswp) { 
    return mix(b64.yx, b64, nswp.xx); // use swizzle version (some device can be slower)
}


#ifdef AMD_F16_BVH
#define FSWP fast16swap
#else
#define FSWP fast32swap
#endif




// BVH utility
uint64_t bitfieldReverse64(in uint64_t a) {uvec2 p = U2P(a);p=bitfieldReverse(p);return P2U(p.yx);}
uvec2 bitfieldReverse64(in uvec2 p) {return bitfieldReverse(p).yx;}

int nlz(in uint64_t x) { return 63 - msb(x); }
int nlz(in uvec2 x) { return 63 - msb(x); }
int nlz(in uint x) { return 31 - msb(x); }
int nlz(in int x) { return nlz(uint(x)); }



// polar/cartesian coordinates
vec2 lcts(in vec3 direct) {
    direct.xyz = direct.xzy * vec3(1.f,1.f,-1.f);
    return vec2(atan(direct.y, direct.x), acos(direct.z));
}

vec3 dcts(in vec2 hr) {
    //return normalize(vec3(cos(hr.x)*sin(hr.y), sin(hr.x)*sin(hr.y), cos(hr.y))).xzy * vec3(1.f,-1.f,1.f);
    return (vec3(cos(hr.x)*sin(hr.y), sin(hr.x)*sin(hr.y), cos(hr.y))).xzy * vec3(1.f,-1.f,1.f);
}




uint p2x_16(in highp uvec2 a) {
#if defined(ENABLE_AMD_INSTRUCTION_SET) && defined(ENABLE_AMD_INT16)
    return packUint2x16(u16vec2(a));
#else
    return (a.x&0xFFFFu)|(a.y<<16u);
#endif
};

highp uvec2 up2x_16(in uint a) {
#if defined(ENABLE_AMD_INSTRUCTION_SET) && defined(ENABLE_AMD_INT16)
    return uvec2(unpackUint2x16(a));
#else
    return uvec2(a&0xFFFFu, a>>16u);
#endif
};


#if defined(ENABLE_AMD_INSTRUCTION_SET) && defined(ENABLE_AMD_INT16)
// it should be 8-bit, but there is no native support
uint p4x_8(in lowp uvec4 a) {return (a.x<<0)|(a.y<<8)|(a.z<<16)|(a.w<<24);};
u16vec4 up4x_8(in uint a) {return u16vec4(a>>0,a>>8,a>>16,a>>24)&0xFFus;};
#else
uint p4x_8(in lowp uvec4 a) {return (a.x<<0)|(a.y<<8)|(a.z<<16)|(a.w<<24);};
lowp uvec4 up4x_8(in uint a) {return uvec4(a>>0,a>>8,a>>16,a>>24)&0xFFu;};
#endif


#define f32_f16 packHalf4x16
#define f16_f32 unpackHalf4x16

// improved generic bilinear interpolation
vec4 textureHQ(in sampler2D SMP, in vec2 TXL, in int LOD) {
    const vec2 sz = textureSize(SMP, LOD), si = 1.f.xx/sz, tx = fma(sz, TXL, -0.5f.xx), lp = fract(tx), tl = si*(ceil(tx)-0.5f.xx);
    const mat4 mtr = mat4(
        textureLodOffset(SMP, tl, LOD, ivec2(0,0)), textureLodOffset(SMP, tl, LOD, ivec2(1,0)),
        textureLodOffset(SMP, tl, LOD, ivec2(0,1)), textureLodOffset(SMP, tl, LOD, ivec2(1,1))
    );
    const vec4 coef = vec4(
        fma( lp.x, lp.y, (1.f-lp.y-lp.x)), // (1.f-lp.x) * (1.f-lp.y)
        fma( lp.x,-lp.y, lp.x),            //      lp.x  * (1.f-lp.y)
        fma(-lp.x, lp.y, lp.y),            // (1.f-lp.x) *      lp.y
        lp.x*lp.y                          //      lp.x  *      lp.y
    );
    return mtr*coef;
};

#endif