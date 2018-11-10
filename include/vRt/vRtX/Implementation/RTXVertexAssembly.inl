#pragma once
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    // construction of extended vertex assembly
    VtResult RTXVertexAssemblyExtension::_Construction(std::shared_ptr<VertexAssemblySet> _assemblySet) {
        VkGeometryTrianglesNV _vertexProxyNV = vk::GeometryTrianglesNV{};
        _vertexProxyNV.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        _vertexProxyNV.vertexOffset = _assemblySet->_verticeBufferCached->_offset();
        _vertexProxyNV.vertexStride = sizeof(float) * 4ull;
        _vertexProxyNV.vertexData = VkBuffer(*_assemblySet->_verticeBufferCached);
        _vertexProxyNV.vertexCount = _assemblySet->_calculatedPrimitiveCount * 3ull;

        // RTX support was broken even there
        _vertexProxyNV.indexType = VK_INDEX_TYPE_NONE_NV;
        _vertexProxyNV.indexCount = _vertexProxyNV.vertexCount;
        
        // index buffer (uint32_t)
        //_vertexProxyNV.indexType = VK_INDEX_TYPE_UINT32;
        //_vertexProxyNV.indexOffset = _assemblySet->_indexBuffer->_offset();
        //_vertexProxyNV.indexData = VkBuffer(*_assemblySet->_indexBuffer);

        VkGeometryDataNV _vertexDataNV = vk::GeometryDataNV{};
        _vertexDataNV.aabbs = vk::GeometryAABBNV{};
        _vertexDataNV.triangles = _vertexProxyNV;

        _vDataNV = vk::GeometryNV{};
        _vDataNV.flags = VK_GEOMETRY_OPAQUE_BIT_NV | VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV;
        _vDataNV.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
        _vDataNV.geometry = _vertexDataNV;

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
