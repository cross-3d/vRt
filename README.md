# vRt

[![Join the chat at https://gitter.im/world8th/vRt](https://badges.gitter.im/world8th/vRt.svg)](https://gitter.im/world8th/vRt?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

> Vulkan API ray-tracing cross-platform layer (like DXR fallback layer). It will supported by common top GPU's (in far future and something else).

## Features and advantages

- Low-level library for ray tracing (compute workflow)
- Much more Vulkan API 1.1 capable (at 2018 year)
- Higher extensibility than original project
- Fixed issues of previous projects
- Reduced overheads, include dispatch calls
- No depends by first GPU (but indev apps may have)
- Support by common modern compilers (Visual Studio 2017, GCC 8.1)
- Support by modern graphics/compute accelerators (NVidia Pascal, AMD Radeon Vega, next-gens...)
- Roundly 100Mrays/s for primary rays in top GPU's (alike RX Vega 64, NVidia Titan Volta)

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

### About old project... 

- [Satellite README](https://github.com/archive-w8th/satellite-deprecated/blob/99c8a86085b45e0a3a6b37e4afdc841510a142d6/README.md) (7 Jun 2018)

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
