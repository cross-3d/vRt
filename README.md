# vRt (VK-1.1.95)

<a href="https://gitter.im/world8th/vRt?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge" target="_blank" title="Gitter"><img src="https://badges.gitter.im/world8th/vRt.svg" alt="Gitter.RT" height="20px"/></a>

> Vulkan API raytracing cross-platform library.

## Tuning for interactivity

### We are so bored! PLEASE, STOP MY SUFFER!

We no longer want to do this bullshit and support this project. It’s easier for us to maintain separate components like radix sorting. For us, the greater pleasure comes more from the way it works, whether it works stably and works in principle. And such a project is difficult to maintain, as it delivers a lot of pain, including when debugging. We realized how important microcontrol is, but we realized that we no longer have time ... It became clear that Hercules’s efforts were achieved by a low code quality, a huge amount of water, and for quick hands with fortunes.

- Too many old or dirty code, needs have so much need hack's and so much time.
- Critical performance drops despite of efforts.
- Enthusiasm drops sharply with awareness and relaxation.
- Error's in different operation systems (such as Linux). 

### Features and advantages

- Low-level library for ray tracing, based on Vulkan API (compute workflow)
- Reduced overheads, single command buffer support, highly optimized
- Support by common modern compilers (Visual Studio 2017, GCC 8.1)
- Support by modern graphics/compute accelerators
- Unified library for desktop dedicated GPU's
- Basic extensions support (WIP)

### Backends

- vRtC (compute-based, native, default, wide GPU support)
- vRtX (NVIDIA RTX only, more higher performance at now)

### Repository mirrors

- https://github.com/world8th/vRt
- https://gitlab.com/world8th/vRt

### Requirements for testing application

- Vulkan API 1.1.92 support 
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- C++17 capable compiler
- GLSL to SPIR-V 1.3 compiler
- CMake 3.12 or higher

### Compatibility Confirmed

- RX Vega 64 (Windows only)
- RTX 2070 (Windows, Ubuntu, RTX On/Off)
- GTX 1070 (early has supported)

### Currently Incompatible... 

- AMDVLK in Linux 
- Mesa/RADV drivers 


### Roadmaps

#### Q1 of 2019

- Major refactoring of code
- Attemping to add Linux support (AMDVLK)
- HLSL shaders for non-core functionality
- Unit tests and documentations
- Alpha stage

#### Indefinite Term

- Begin of development for OpenGL, OpenCL and CUDA interoperability
- First implementations for games, engines, applications, blockchains
- Make custom GLSL/HLSL compiler for unified compatibility
- Support of PowerVR Wizard Ray Tracing, Intel Larrabee 2019

### Known Issues

- No support of Mesa drivers directly. 
- In AMDVLK doesn't working, we doesn't know why.
- Unable to run in NVIDIA GPU's, need use this version of driver (https://developer.nvidia.com/vulkan-driver)
- Intel HD Graphics UHD 630 doesn't work. Intel integrated graphics still needs to workarounds.
- Not working some meshes. We still do search the solutions for this problem.

