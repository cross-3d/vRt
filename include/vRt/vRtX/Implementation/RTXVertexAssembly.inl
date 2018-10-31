#pragma once
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    // construction of extended vertex assembly
    VtResult RTXVertexAssemblyExtension::_Construction(std::shared_ptr<VertexAssemblySet> _assemblySet) {
        VkGeometryTrianglesNVX _vertexProxyNVX = vk::GeometryTrianglesNVX{};
        _vertexProxyNVX.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        _vertexProxyNVX.vertexOffset = _assemblySet->_verticeBufferCached->_offset();
        _vertexProxyNVX.vertexStride = sizeof(float) * 4ull;
        _vertexProxyNVX.vertexData = VkBuffer(*_assemblySet->_verticeBufferCached);
        _vertexProxyNVX.vertexCount = _assemblySet->_calculatedPrimitiveCount * 3ull;

        _vertexProxyNVX.indexType = VK_INDEX_TYPE_UINT32;
        _vertexProxyNVX.indexCount = _assemblySet->_calculatedPrimitiveCount * 3ull;
        _vertexProxyNVX.indexOffset = _assemblySet->_indexBuffer->_offset();
        _vertexProxyNVX.indexData = VkBuffer(*_assemblySet->_indexBuffer);

        VkGeometryDataNVX _vertexDataNVX = vk::GeometryDataNVX{};
        _vertexDataNVX.aabbs = vk::GeometryAABBNVX{};
        _vertexDataNVX.triangles = _vertexProxyNVX;

        _vDataNVX = vk::GeometryNVX{};
        _vDataNVX.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NVX;
        _vDataNVX.geometry = _vertexDataNVX;

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
