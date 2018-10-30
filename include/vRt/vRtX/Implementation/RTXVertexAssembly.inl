#pragma once
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    // construction of extended vertex assembly
    VtResult RTXVertexAssemblyExtension::_Construction(std::shared_ptr<VertexAssemblySet> _assemblySet) {
        _vertexProxyNVX = vk::GeometryTrianglesNVX{};
        _vertexProxyNVX.vertexFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        _vertexProxyNVX.vertexOffset = 0ull;
        _vertexProxyNVX.vertexStride = sizeof(float) * 4ull;
        _vertexProxyNVX.vertexData = *_assemblySet->_verticeBufferCached;
        _vertexProxyNVX.vertexCount = _assemblySet->_calculatedPrimitiveCount;
        return VK_SUCCESS;
        //return VK_ERROR_EXTENSION_NOT_PRESENT;
    };

    // helper for connection
    VtResult RTXAcceleratorExtension::_ConstructVertexAssembly(std::shared_ptr<VertexAssemblySet> assemblySet) {
        auto assemblySetExt = std::make_shared<RTXVertexAssemblyExtension>();
        assemblySet->_hExtension = assemblySetExt;
        return assemblySetExt->_Construction(assemblySet);
    };

};
