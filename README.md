# vRt (VK-1.1.91)

<a href="https://gitter.im/world8th/vRt?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge" target="_blank" title="Gitter"><img src="https://badges.gitter.im/world8th/vRt.svg" alt="Gitter.RT" height="20px"/></a>
<a href="https://etherdonation.com/d?to=0xd9b022cdf82ee7eab9b17c7d85c3ba005e288383" target="_blank" title="Donate ETH"><img src="https://etherdonation.com/i/btn/donate-btn.png" alt="Donate.ETH" height="20px"/></a>

> Vulkan API raytracing cross-platform library.

## Tuning for interactivity

### Features and advantages

- Low-level library for ray tracing, based on Vulkan API (compute workflow)
- Reduced overheads, single command buffer support, highly optimized
- Support by common modern compilers (Visual Studio 2017, GCC 8.1)
- Support by modern graphics/compute accelerators
- Can reach 100Mrays/s in top GPU's (without native or specific hardware acceleration)
- Unified library for desktop dedicated GPU's
- Extensions for accelerations

### Repository mirrors

- https://github.com/world8th/vRt
- https://gitlab.com/world8th/vRt

### Requirements for testing application

- Vulkan API 1.1.82 support (1.1.92 needs for RTX-based support)
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- C++17 capable compiler
- GLSL to SPIR-V 1.3 compiler
- CMake 3.12 or higher

### Compatibility Confirmed

- RX Vega 64
- RTX 2070 (without RTX acceleration)
- GTX 1070 (need new tests)

### Roadmaps

#### November 2018

- Advanced ray-tracing API (support of memory sharing, custom pipelines)
- Better memory managments (use transfer buffers instead of cmdUpdateBuffer)
- Reduce API bloating (disable useless VtDevice traffic buffers)
- Public MVP and prototype stage (with powerful performance)

#### Q1 of 2019

- HLSL shaders for non-core functionality
- Unit tests and documentations
- Alpha stage

#### Indefinite Term

- Support RTX acceleration and next-gen NVidia GPU's (awaiting in 2023 year)
- Begin of development for OpenGL, OpenCL and CUDA interoperability
- First implementations for games, engines, applications, blockchains
- Make custom GLSL/HLSL compiler for unified compatibility
- Support of PowerVR Wizard Ray Tracing, Intel Larrabee 2019

### Known Issues

- Implementation, based on `VK_NV_ray_tracing` not working. NVIDIA just added stubs only, in practice is useless now. Nothing any news about drivers (https://developer.nvidia.com/vulkan-driver).
- Not enough performance on RX Vega 64 (where is promised millions, why we have only half?). We attemped to restore lost performances, but still need very deep work to full remedy of application architecture.
- Intel HD Graphics UHD 630 doesn't work. Intel integrated graphics still needs to workarounds.
- Not working some meshes. Still need full application rework to make better memory managments.
