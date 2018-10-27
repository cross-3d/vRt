# vRt (VK-1.1.88)

<a href="https://gitter.im/world8th/vRt?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge" target="_blank" title="Gitter"><img src="https://badges.gitter.im/world8th/vRt.svg" alt="Gitter.RT" height="20px"/></a>
<a href="https://etherdonation.com/d?to=0xd9b022cdf82ee7eab9b17c7d85c3ba005e288383" target="_blank" title="Donate ETH"><img src="https://etherdonation.com/i/btn/donate-btn.png" alt="Donate.ETH" height="20px"/></a>

> Unified Vulkan API ray tracing cross-platform library.

## Features and advantages

- Low-level library for ray tracing, based on Vulkan API (compute workflow)
- Reduced overheads, single command buffer support, highly optimized
- Support by common modern compilers (Visual Studio 2017, GCC 8.1)
- Support by modern graphics/compute accelerators (NVidia Pascal, AMD Radeon Vega, next-gens...)
- Can reach 100Mrays/s in top GPU's (without native or specific hardware acceleration)
- Unified library for desktop dedicated GPU's

## Repository mirrors

- https://github.com/world8th/vRt
- https://gitlab.com/world8th/vRt

## Requirements

- Vulkan API 1.1.82 support
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- C++17 capable compiler
- GLSL to SPIR-V 1.3 compiler
- CMake 3.12 or higher

## Compatibility Confirmed

- RX Vega 64
- GTX 1070 (need new tests)

## Uncomfirmed

- RTX 2070, 2080, 2080 Ti (non-RTX mode only)
- GTX 1080, 1080 Ti, 980, 980 Ti
- RX Vega 56 and Raven Ridge
- RX 580, 570 series

## Roadmaps

**November 2018**

- Support for RTX graphics cards (RTX 2070, 2080, etc.)
- HLSL shaders for non-core functionality
- Initial support of extensions
- Advanced ray-tracing API (i.e. reduce API bloating)
- Public MVP and prototype stage

**Q1 of 2019**

- Unit tests 
- Documentation 
- Alpha stage

**Indefinite Term**

- Begin of development for OpenGL, OpenCL and CUDA interoperability
- First implementations for games, engines, applications, blockchains
- Make custom GLSL/HLSL compiler for unified compatibility
- Support of PowerVR Wizard Ray Tracing, Intel Larrabee 2019
