# vRt

[![Join the chat at https://gitter.im/world8th/vRt](https://badges.gitter.im/world8th/vRt.svg)](https://gitter.im/world8th/vRt?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## What is it?
> Vulkan API ray-tracing cross-platform layer (like DXR fallback layer). It will supported by common top GPU's (in far future and something else).

## Features and advantages
- Low level API for ray tracing (even lower than Embree)
- Much more Vulkan API 1.1 capable (at 2018 year)
- Higher extensibility than original project
- Fixed issues of previous projects 
- Reduced overheads, include dispatch calls 

## Silver box...
- Around 96 million rays per second (more than old project)
- Around (up to) 120Hz in Sponza scene (RX Vega 64, 1280x720, without rebuilding BVH, primary rays, no sorting)

## Repositories:
- https://github.com/world8th/vRt (GitHub, owned by Microsoft)
- https://gitlab.com/world8th/vRt (GitLab, merging)

## Requirements: 
- Vulkan API 1.1 headers and libraries
- Vulkan Memory Allocator, VMA (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- C++17 capable compiler (include Visual Studio 2017)
- GLSL to SPIR-V 1.3 compiler
- GLM for test application run

## Current API: 
You can see here current API:
- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/API.inl
- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/Structures.inl
- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/Enums.inl

## Current Limitations:
- No support of top level BVH (only bottom level at moment)
- No common DXR features 

We still working on forming of ray tracing API. 

## VERY Relative Calendar (VRC):

#### July 2018 ("Hydrogen")
- Development of better implementation 
- Improve API and interfaces
- Support for NVidia (GTX 9, 10, Titan series) and AMD (RX Vega) hardware

#### 2018 Q4 ("Lithium")
- Coming to (pre-)alpha stage
- Finish to fix (un)common bugs and issues 
- Write bigger real-time application with ray tracing 
- Make support of all Vulkan API 1.1 (or newer) supported platforms

#### Beyond...  
- First implementations in bigger framework and engines (The Forge, Unreal Engine 5, Blender 2.90)
- Make custom GLSL/HLSL compiler for unified compatibility 
- Coming to beta stage
- Compatibility with Nvidia RTX, PowerVR Wizard Ray Tracing, Intel Larrabee 2019
- Compatibility with mobile platform 
- Compatibility with Apple OS
