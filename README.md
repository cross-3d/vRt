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

## Repositories:
- https://github.com/world8th/vRt (GitHub, owned by Microsoft)
- https://gitlab.com/world8th/vRt (GitLab, merging)

## Requirements: 
- Vulkan API 1.1 headers and libraries
- Vulkan Memory Allocator, VMA (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- C++17 capable compiler (include Visual Studio 2017)
- GLSL to SPIR-V 1.3 compiler
- GLM for test application run

## VERY Relative Calendar (VRC):

#### June/July 2018 ("Early Vulkanism")
- Debugging and experimenting stage
- Development of better implementation 
- Improve API and interfaces
- Support for NVidia (GTX 9, 10, Titan series) and AMD (RX Vega) hardware

#### August 2018 ("First Life"), hope
- Coming to alpha stage
- Finish to fix common bugs and issues 
- First implementations in bigger framework and engines (The Forge, Unreal Engine 5, Blender 2.90)
- Write real-time application with ray tracing 

#### Beyond... 
- Make custom GLSL/HLSL compiler for unified compatibility 
- Coming to beta stage
- Compatibility with Nvidia RTX, PowerVR Wizard Ray Tracing, Intel Larrabee 2019
- Compatibility with mobile platform 
- Compatibility with Apple OS
