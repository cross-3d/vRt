#ifndef _VERTEX_INPUT_H
#define _VERTEX_INPUT_H


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



// input data of vertex instance
//layout ( binding = 0, set = 1, std430 ) readonly buffer bufferSpaceB {INDEX16 bufferSpace[]; };
layout ( binding = 0, set = 1 ) uniform usamplerBuffer bufferSpace[8]; // vertex model v1.2
//layout ( binding = 1, set = 1, std430 ) readonly buffer VT_BUFFER_REGION {VtBufferRegion bufferRegions[]; };
layout ( binding = 2, set = 1, std430 ) readonly buffer VT_BUFFER_VIEW {VtBufferView bufferViews[]; };
layout ( binding = 3, set = 1, std430 ) readonly buffer VT_ACCESSOR {VtAccessor accessors[]; };
layout ( binding = 4, set = 1, std430 ) readonly buffer VT_ATTRIB {VtAttributeBinding attributes[]; };


// uniform input of vertex loader
#ifdef USE_PUSH_CONSTANT
layout ( push_constant ) uniform VT_UNIFORM 
#else
layout ( binding = 5, set = 1, std430 ) readonly buffer VT_UNIFORM
#endif
{
    uint primitiveCount;
    uint verticeAccessor;
    uint indiceAccessor;
    uint materialID;

    uint primitiveOffset;
    uint topology;
    uint attributeCount;
    uint reserved0;
} vertexBlock;



uint calculateByteOffset(in int accessorID, in uint index, in uint bytecorrect){
    int bufferView = accessors[accessorID].bufferView, bufferID = bufferViews[bufferView].regionID;
    uint offseT = 0u;//bufferRegions[bufferID].byteOffset;
    offseT += bufferViews[bufferView].byteOffset + accessors[accessorID].byteOffset; // calculate byte offset 
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
        if (C >= 1) outp.x = uintBitsToFloat(M32(bufferSpace[bufferID],T+0));
        if (C >= 2) outp.y = uintBitsToFloat(M32(bufferSpace[bufferID],T+1));
        if (C >= 3) outp.z = uintBitsToFloat(M32(bufferSpace[bufferID],T+2));
        if (C >= 4) outp.w = uintBitsToFloat(M32(bufferSpace[bufferID],T+3));
    }
}

// vec3 getter
void readByAccessor(in int accessor, in uint index, inout vec3 outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        uint C = aComponents(accessors[accessor].bitfield)+1;
        if (C >= 1) outp.x = uintBitsToFloat(M32(bufferSpace[bufferID],T+0));
        if (C >= 2) outp.y = uintBitsToFloat(M32(bufferSpace[bufferID],T+1));
        if (C >= 3) outp.z = uintBitsToFloat(M32(bufferSpace[bufferID],T+2));
    }
}

// vec2 getter
void readByAccessor(in int accessor, in uint index, inout vec2 outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        uint C = aComponents(accessors[accessor].bitfield)+1;
        if (C >= 1) outp.x = uintBitsToFloat(M32(bufferSpace[bufferID],T+0));
        if (C >= 2) outp.y = uintBitsToFloat(M32(bufferSpace[bufferID],T+1));
    }
}

// float getter
void readByAccessor(in int accessor, in uint index, inout float outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        outp = uintBitsToFloat(M32(bufferSpace[bufferID],T+0));
    }
}

// int getter
void readByAccessor(in int accessor, in uint index, inout int outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        uint T = calculateByteOffset(accessor, index, 2);
        outp = int(M32(bufferSpace[bufferID],T+0));
    }
}

// planned read type directly from accessor
void readByAccessorIndice(in int accessor, in uint index, inout uint outp) {
    if (accessor >= 0) {
        int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        const bool U16 = aType(accessors[accessor].bitfield) == 2; // uint16
        uint T = calculateByteOffset(accessor, index, U16 ? 1 : 2);
        if (U16) { outp = M16(bufferSpace[bufferID],T+0); } else { outp = M32(bufferSpace[bufferID],T+0); }
    }
}

#endif