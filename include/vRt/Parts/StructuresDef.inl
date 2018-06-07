#pragma once
#include "Headers.inl"
#include "StructuresLow.inl"
//#include "HandlersDef.inl"
//#include "Enums.inl"

namespace vt { // store in official namespace

    // in general that is conversion
    struct VtInstanceConversionInfo;
    struct VtArtificalDeviceExtension;
    struct VtDeviceConversionInfo;
    struct VtPhysicalDeviceConversionInfo;
    struct VtRayTracingPipelineCreateInfo;

    // use immutables in accelerator inputs
    struct VtVertexInputCreateInfo;

    // use as low level typed descriptor set
    struct VtMaterialSetCreateInfo;
    struct VtAcceleratorCreateInfo;


    // custom (unified) object create info, exclusive for vRt ray tracing system, and based on classic Satellite objects
    // bound in device space
    // planned to add support of HostToGpu and GpuToHost objects 

    struct VtDeviceBufferCreateInfo;
    struct VtDeviceImageCreateInfo;

};
