# vRt

[![Join the chat at https://gitter.im/world8th/vRt](https://badges.gitter.im/world8th/vRt.svg)](https://gitter.im/world8th/vRt?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

> Vulkan API ray-tracing cross-platform layer (like DXR fallback layer). It will supported by common top GPU's (in far future and something else).

## Features and advantages

- Low level API for ray tracing (even lower than Embree)
- Much more Vulkan API 1.1 capable (at 2018 year)
- Higher extensibility than original project
- Fixed issues of previous projects
- Reduced overheads, include dispatch calls

## Silver box

- Around 96 million rays per second (more than old project)
- Around (up to) 90Hz in Sponza scene (RX Vega 64, 1280x720, without rebuilding BVH, primary rays, no sorting)

## Repositories

- https://github.com/world8th/vRt (GitHub, owned by Microsoft)
- https://gitlab.com/world8th/vRt (GitLab, merging)

## Requirements

- Vulkan API 1.1 headers and libraries
- Vulkan Memory Allocator, VMA (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- C++17 capable compiler
- GLSL to SPIR-V 1.3 compiler
- GLM for test application run

## Compatibility confirmed

- RX Vega 64 (Windows 10)
- GTX 1070 (Windows 10, need newer tests)

## Current API

You can see here current API:

- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/API.inl
- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/Structures.inl
- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/Enums.inl

We still working on forming of ray tracing API.

## Roadmaps

### August 2018 ("Helium")

- Add V-EZ support, add interoperability with V-EZ by GPUOpen
- Make support of all Vulkan API 1.1 (or newer) supported platforms
- Add texturing, materials and PBR support and example
- Make example renderer, based on hybrid ray tracing
- Add controllers, update all possible libraries
- Add reflections and shadows examples

### 2018 Q4 ("Lithium")

- Coming to alpha stage
- Finish to fix (un)common bugs and issues
- Write bigger real-time application with ray tracing

### Beyond

- First implementations in bigger framework and engines (The Forge, Unreal Engine 5, Blender 2.90)
- Make custom GLSL/HLSL compiler for unified compatibility 
- Coming to beta stage
- Compatibility with Nvidia RTX (Volta/Turing), PowerVR Wizard Ray Tracing (GR6500), AMD Radeon RXL (Navi, next-gen), Intel Larrabee 2019
- Compatibility with mobile platform
- Compatibility with Apple OS
