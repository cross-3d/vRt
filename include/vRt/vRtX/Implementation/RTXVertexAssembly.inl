#pragma once
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    // construction of extended vertex assembly
    VtResult RTXVertexAssemblyExtension::_Construction(std::shared_ptr<VertexAssemblySet> _assemblySet) {
        VkGeometryTrianglesNV _vertexProxyNVX = vk::GeometryTrianglesNV{};
        _vertexProxyNVX.vertexFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        _vertexProxyNVX.vertexOffset = _assemblySet->_verticeBufferCached->_offset();
        _vertexProxyNVX.vertexStride = sizeof(float) * 4ull;
        _vertexProxyNVX.vertexData = VkBuffer(*_assemblySet->_verticeBufferCached);
        _vertexProxyNVX.vertexCount = _assemblySet->_calculatedPrimitiveCount * 3ull;

        _vertexProxyNVX.indexType = VK_INDEX_TYPE_UINT32;
        _vertexProxyNVX.indexCount = _assemblySet->_calculatedPrimitiveCount * 3ull;
        _vertexProxyNVX.indexOffset = _assemblySet->_indexBuffer->_offset();
        _vertexProxyNVX.indexData = VK_NULL_HANDLE; //VkBuffer(*_assemblySet->_indexBuffer);

        VkGeometryDataNV _vertexDataNVX = vk::GeometryDataNV{};
        _vertexDataNVX.aabbs = vk::GeometryAABBNV{};
        _vertexDataNVX.triangles = _vertexProxyNVX;

        _vDataNV = vk::GeometryNV{};
        _vDataNV.flags = VK_GEOMETRY_OPAQUE_BIT_NV | VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV;
        _vDataNV.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
        _vDataNV.geometry = _vertexDataNVX;

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
