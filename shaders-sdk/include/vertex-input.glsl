#ifndef _VERTEX_INPUT_H
#define _VERTEX_INPUT_H


#ifdef VERTEX_FILLING
layout ( binding = 3, set = VTX_SET, align_ssbo ) coherent buffer VTX_BUFFER_IN { vec4 data[]; } lvtxIn[];
#endif


// buffer region
struct VtBufferRegion {
    uint byteOffset;
    uint byteSize;
};

// subdata structuring in buffer region
struct VtBufferView {
    int regionID;
    int byteOffset; // in structure offset
    int byteStride;
    int byteLength;
};

// accessor
struct VtAccessor {
    int bufferView; // buffer-view structure
    int byteOffset; // accessor byteOffset
    uint bitfield; // VtFormat decodable
    uint reserved;
};

// attribute binding
struct VtAttributeBinding {
    int attributeID;
    int accessorID;
    //int indexOffset;
};



const ivec2 COMPONENTS = ivec2(0, 2);
const ivec2 ATYPE = ivec2(2, 4);
const ivec2 NORMALIZED = ivec2(6, 1);

int aComponents(in uint bitfield) { return int(parameteri(COMPONENTS, bitfield)); };
int aNormalized(in uint bitfield) { return int(parameteri(NORMALIZED, bitfield)); };
int aType(in uint bitfield) { return int(parameteri(ATYPE, bitfield)); };


#define BFS uint(bufferID)

//#ifdef ENABLE_INT16_SUPPORT
//layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { u16vec2 data[]; } bufferSpace[];
//#else
//layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { uint data[]; } bufferSpace[];
//#endif

//layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { uint8_t data[]; } bufferSpace[];
//#ifdef ENABLE_INT16_SUPPORT
  layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { uint8_t data[]; } bufferSpace[];
//#else
//layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { uint     data[]; } bufferSpace[]; // legacy GPU's
//#endif

layout ( binding = 2, set = 1, align_ssbo ) readonly buffer VT_BUFFER_VIEW { VtBufferView bufferViews[]; };
layout ( binding = 3, set = 1, align_ssbo ) readonly buffer VT_ACCESSOR { VtAccessor accessors[]; };
layout ( binding = 4, set = 1, align_ssbo ) readonly buffer VT_ATTRIB { VtAttributeBinding attributes[]; };

// alias for 32-bit packed
#define u32x1_t uint32_t
#define p32x1_t(a) p32x1_k(u16x2_t(a))

// uint16_t support to read (extended)
#ifdef ENABLE_INT16_SUPPORT
#define u16x1_t uint16_t
#define u16x4_t u16vec4
#define u16x2_t u16vec2
#define p32x1_k pack32
#define lowq  
#else
#define u16x1_t uint
#define u16x4_t uvec4
#define u16x2_t uvec2
#define lowq lowp
u32x1_t p32x1_k (in highp u16x2_t a) { return (a.y<<16u)|a.x; };
#endif

// uint8_t support to read
#ifdef ENABLE_NATIVE_U8
#define u8x4_t u8vec4
#define u8x2_t u8vec2
#define u8x4pack pack32
#define u8x2pack pack16
#else
#define u8x4_t u16x4_t
#define u8x2_t u16x2_t
const lowq u8x2_t bshift16 = {0u,8u};
const lowq u8x4_t bshift32 = bshift16.xyxy;
u32x1_t u8x4pack(in lowq u8x4_t v4) { v4 <<= bshift32; return p32x1_t(v4.xz|v4.yw); };
u16x1_t u8x2pack(in lowq u8x2_t v2) { v2 <<= bshift16; return u16x1_t(v2[0]|v2[1]); };
#endif

// First in world ByteAddressBuffer in Vulkan API by Ispanec (tested in RTX 2070 only)
u16x1_t M16(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI; return u8x2pack(u8x2_t(bufferSpace[BSC].data[Ot+0u],bufferSpace[BSC].data[Ot+1u])); };
u32x1_t M32(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI; return u8x4pack(u8x4_t(bufferSpace[BSC].data[Ot+0u],bufferSpace[BSC].data[Ot+1u],bufferSpace[BSC].data[Ot+2u],bufferSpace[BSC].data[Ot+3u])); };


//#ifdef ENABLE_INT16_SUPPORT // 16-bit wide version, optimized (with RX Vega support) 
//uint16_t M16(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=1; return uint16_t(bufferSpace[BSC].data[Ot+0u]); };
//uint32_t M32(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=1; return pack32(u16vec2(bufferSpace[BSC].data[Ot+0u],bufferSpace[BSC].data[Ot+1u])); };
//#else // for legacy GPU support
//const lowp ivec2 b16m = {0,16};
//highp uint M16(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=1; return bitfieldExtract(bufferSpace[BSC].data[Ot>>1u],b16m[Ot&1u],16); };
//      uint M32(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=2; return bufferSpace[BSC].data[Ot]; };
//#endif


struct VtVIUniform {
    uint primitiveCount;
    int verticeAccessor;
    int indiceAccessor;
    int materialAccessor;

    uint primitiveOffset;
    uint attributeOffset;
    uint attributeCount;
    uint bitfield;

    uint materialID;
    uint readOffset;
    uint reserved0, reserved1;
};


// uniform input of vertex loader
layout ( binding = 9, set = VTX_SET, align_ssbo ) readonly buffer VT_UNIFORM { VtVIUniform _vertexBlock[]; };
layout ( push_constant ) uniform VT_CONSTS { uint inputID; } cblock;
#define vertexBlock _vertexBlock[gl_GlobalInvocationID.y + cblock.inputID]
layout ( binding = 6, set = 1, align_ssbo ) readonly buffer VT_TRANSFORMS { mat3x4 vTransforms[]; };


uint calculateByteOffset(in int accessorID, in const uint bytecorrect) { //bytecorrect -= 1;
    const uint bufferView = uint(accessors[accessorID].bufferView);
    return ((bufferViews[bufferView].byteOffset+accessors[accessorID].byteOffset) >> bytecorrect);
};

uint iCR(in int accessorID, in uint index, in const uint cmpc, in const uint bytecorrect) {
    const uint bufferView = uint(accessors[accessorID].bufferView), stride = max(bufferViews[bufferView].byteStride, (aComponents(accessors[accessorID].bitfield)+1)<<bytecorrect);
    return (index*stride)+(cmpc<<bytecorrect);
};

void readByAccessorLL(in int accessor, in uint index, inout uvec4 outpx) {
    [[flatten]] if (accessor >= 0) {
        const int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        const uint T = calculateByteOffset(accessor, 0u), D = 0u, C = min(aComponents(accessors[accessor].bitfield)+1, 4u-D);
        [[unroll]] for (int i=0;i<4;i++) { [[flatten]] if (C > i) outpx[D+i] = M32(BFS,T,iCR(accessor,index,i,2u)); };
    };
};

// 
uvec4 readByAccessorLLW(in int accessor, in uint index, in uvec4 outpx) { readByAccessorLL(accessor, index, outpx); return outpx; };

// vec4 getter
void readByAccessor(in int accessor, in uint index, inout vec4 outp) {
    outp.xyzw = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(outp)).xyzw);
};

// vec3 getter
void readByAccessor(in int accessor, in uint index, inout vec3 outp) {
    outp.xyz = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.x))).xyz);
};

// vec2 getter
void readByAccessor(in int accessor, in uint index, inout vec2 outp) {
    outp.xy = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.xx))).xy);
};

// vec1 getter
void readByAccessor(in int accessor, in uint index, inout float outp) {
    outp.x = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.xxx))).x);
};

// ivec1 getter
void readByAccessor(in int accessor, in uint index, inout int outp) {
    outp.x = int(readByAccessorLLW(accessor, index, uvec4(outp, 0u.xxx)).x);
};

// uvec1 getter
void readByAccessor(in int accessor, in uint index, inout uint outp) {
    outp.x = readByAccessorLLW(accessor, index, uvec4(outp, 0u.xxx)).x;
};


// planned read type directly from accessor
void readByAccessorIndice(in int accessor, in uint index, inout uint outp) {
    [[flatten]] if (accessor >= 0) {
        const int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        const bool U16 = aType(accessors[accessor].bitfield) == 2; // uint16
        const uint T = calculateByteOffset(accessor, 0u);
        [[flatten]] if (U16) { outp = M16(BFS,T,iCR(accessor,index,0,1u)); } else { outp = M32(BFS,T,iCR(accessor,index,0,2u)); };
    };
};

// 
void storePosition(in ivec2 cdata, in vec4 fval) {
    const uint inputID = gl_GlobalInvocationID.y + uint(cblock.inputID);
    fval.xyz = mult4(vTransforms[inputID], fval);
    lvtxIn[0].data[cdata.x*3+cdata.y] = fval;
};


#endif
