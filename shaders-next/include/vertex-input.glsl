#ifndef _VERTEX_INPUT_H
#define _VERTEX_INPUT_H







#ifdef ENABLE_AMD_INSTRUCTION_SET
uint16_t M16(in f16samplerBuffer m, in uint i) {
    const u16vec2 mpc = float16BitsToUint16(texelFetch(m, int(i>>1)).xy);
    return (i&1)==1?mpc.y:mpc.x;
}

uint M32(in f16samplerBuffer m, in uint i) { 
    return packFloat2x16(texelFetch(m, int(i)).xy);
}
#endif

highp uint M16(in highp usamplerBuffer m, in uint i) {
    const highp uvec2 mpc = texelFetch(m, int(i>>1)).xy;
    return (i&1)==1?mpc.y:mpc.x;
}

uint M32(in highp usamplerBuffer m, in uint i) {
    const highp uvec2 mpc = texelFetch(m, int(i)).xy;
    return ((mpc.y<<16u)|mpc.x);
}


highp uint M16(in mediump samplerBuffer m, in uint i) {
    const highp uvec2 mpc = floatBitsToUint(texelFetch(m, int(i>>1)).xy);
    return (i&1)==1?mpc.y:mpc.x;
}

uint M32(in mediump samplerBuffer m, in uint i) {
    //return packHalf2x16(texelFetch(m, int(i)).xy); // inaccurate 
    const highp uvec2 mpc = floatBitsToUint(texelFetch(m, int(i)).xy);
    return ((mpc.y<<16u)|mpc.x);
}


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

int aComponents(in uint bitfield) {
    return parameteri(COMPONENTS, bitfield);
}

int aType(in uint bitfield) {
    return parameteri(ATYPE, bitfield);
}

int aNormalized(in uint bitfield) {
    return parameteri(NORMALIZED, bitfield);
}



#define BFS bufferSpace[nonuniformEXT(bufferID)]



#ifdef ENABLE_AMD_INSTRUCTION_SET
layout ( binding = 0, set = 1 ) uniform f16samplerBuffer bufferSpace[8]; // vertex model v1.4
#else
layout ( binding = 0, set = 1 ) uniform highp usamplerBuffer bufferSpace[8]; // vertex model v1.4
#endif

//layout ( binding = 0, set = 1 ) uniform mediump samplerBuffer bufferSpace[8];

layout ( binding = 2, set = 1, std430 ) readonly buffer VT_BUFFER_VIEW {VtBufferView bufferViews[]; };
layout ( binding = 3, set = 1, std430 ) readonly buffer VT_ACCESSOR {VtAccessor accessors[]; };
layout ( binding = 4, set = 1, std430 ) readonly buffer VT_ATTRIB {VtAttributeBinding attributes[]; };



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
layout ( binding = 5, set = 1, std430 ) readonly buffer VT_UNIFORM { VtVIUniform _vertexBlock[]; };
layout ( push_constant ) uniform VT_CONSTS { uint inputID; } cblock;
#define vertexBlock _vertexBlock[gl_GlobalInvocationID.y + cblock.inputID]



uint calculateByteOffset(in int accessorID, in uint index, in uint bytecorrect) { //bytecorrect -= 1;
    int bufferView = accessors[accessorID].bufferView;
    uint offseT = bufferViews[bufferView].byteOffset + accessors[accessorID].byteOffset; // calculate byte offset 
    uint stride = max(bufferViews[bufferView].byteStride, (aComponents(accessors[accessorID].bitfield)+1) << bytecorrect); // get true stride 
    offseT += index * stride; // calculate structure indexed offset
    return offseT >> bytecorrect;
}


// vec4 getter
void readByAccessor(in int accessor, in uint index, inout vec4 outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        uint C = aComponents(accessors[accessor].bitfield)+1;
        [[flatten]]
        if (C >= 1) outp.x = uintBitsToFloat(M32(BFS,T+0));
        [[flatten]]
        if (C >= 2) outp.y = uintBitsToFloat(M32(BFS,T+1));
        [[flatten]]
        if (C >= 3) outp.z = uintBitsToFloat(M32(BFS,T+2));
        [[flatten]]
        if (C >= 4) outp.w = uintBitsToFloat(M32(BFS,T+3));
    }
}

// vec3 getter
void readByAccessor(in int accessor, in uint index, inout vec3 outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        uint C = aComponents(accessors[accessor].bitfield)+1;
        [[flatten]]
        if (C >= 1) outp.x = uintBitsToFloat(M32(BFS,T+0));
        [[flatten]]
        if (C >= 2) outp.y = uintBitsToFloat(M32(BFS,T+1));
        [[flatten]]
        if (C >= 3) outp.z = uintBitsToFloat(M32(BFS,T+2));
    }
}

// vec2 getter
void readByAccessor(in int accessor, in uint index, inout vec2 outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        uint C = aComponents(accessors[accessor].bitfield)+1;
        [[flatten]]
        if (C >= 1) outp.x = uintBitsToFloat(M32(BFS,T+0));
        [[flatten]]
        if (C >= 2) outp.y = uintBitsToFloat(M32(BFS,T+1));
    }
}

// float getter
void readByAccessor(in int accessor, in uint index, inout float outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        outp = uintBitsToFloat(M32(BFS,T+0));
    }
}

// int getter
void readByAccessor(in int accessor, in uint index, inout int outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        outp = int(M32(BFS,T+0));
    }
}

// planned read type directly from accessor
void readByAccessorIndice(in int accessor, in uint index, inout uint outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        const bool U16 = aType(accessors[accessor].bitfield) == 2; // uint16
        uint T = calculateByteOffset(accessor, index, U16 ? 1 : 2);
        [[flatten]]
        if (U16) { outp = M16(BFS,T+0); } else { outp = M32(BFS,T+0); }
    }
}

#endif
