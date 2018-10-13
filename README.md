# vRt (VK1.1)

<a href="https://gitter.im/world8th/vRt?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge" target="_blank" title="Gitter"><img src="https://badges.gitter.im/world8th/vRt.svg" alt="Donate ETH" height="20px"/></a>
<a href="https://etherdonation.com/d?to=0xd9b022cdf82ee7eab9b17c7d85c3ba005e288383" target="_blank" title="Donate ETH"><img src="https://etherdonation.com/i/btn/donate-btn.png" alt="Donate ETH" height="20px"/></a>

> Unified Vulkan API ray tracing cross-platform library.

## Features and advantages

- Low-level library for ray tracing, based on Vulkan API (compute workflow)
- Fixed issues of previous projects, added sort of new features
- Reduced overheads, single command buffer support, highly optimized
- Support by common modern compilers (Visual Studio 2017, GCC 8.1)
- Support by modern graphics/compute accelerators (NVidia Pascal, AMD Radeon Vega, next-gens...)
- Can reach 100Mrays/s in top GPU's (tested RX Vega 64, in sponza scene, fully dynamic mode)
- Support 3D dimensional space (by default)

## Repository mirrors

- https://github.com/world8th/vRt
- https://gitlab.com/world8th/vRt

## Requirements

- Vulkan API 1.1 Support
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- C++17 capable compiler
- GLSL to SPIR-V 1.3 compiler

## Compatibility Confirmed

- RX Vega 64
- GTX 1070

## Uncomfirmed

- RTX 2080, 2080 Ti (non-RTX mode only)
- GTX 1070 Ti, 1080 Ti (Pascal), 980, 980 Ti (Maxwell)
- RX Vega 56 and Raven Ridge
- RX 580, 480 (optimizations ready only for RX Vega 64 and that series)

## Roadmaps

**October 2018**

- De-vendorize possible features
- Refactoring code and adding minor features
- Better compute and shading  

**November 2018**

- Support for RTX graphics cards (RTX 2070, 2080, etc.)
- Initial support of extensions (which will based on RTX acceleration)
- Advanced ray-tracing API (include top level BVH)
- HLSL shaders for non-core functionality

**January 2019** (or beyond)

- Extended RTX support (with acceleration)
- Coming to alpha stage

**Indefinite Term**

- Unit tests
- Begin of development for OpenGL, OpenCL and CUDA interoperability
- First implementations for games, engines, applications, blockchains
- Make custom GLSL/HLSL compiler for unified compatibility
- Support of PowerVR Wizard Ray Tracing, Intel Larrabee 2019

### About RTX support

We have no so much priorities about support of RTX series, since NVidia giving so huge price to video cards, and not no many people can order now... We will more active, when pricing will down to 399$. Yes, we can collect that funds, but will not so much users at this moment. For first times, we may suggest get early access by donation (but it is not exactly), but after all we can give public accesses. Our priorities now are just to ensure compatibility.

## Direct Donation (ETH)

For development of RTX back-end, need around 700$ for purchase NVidia RTX 2080 series card... You can donate for us.

In order to avoid charges of fraud, I will say directly ... Yes, I can not guarantee you a refund, I do not guarantee that this product will be useful for you, I do not guarantee any benefits to you, I can not guarantee that I will meet your expectations. By sending us a donation, you take your risk and say goodbye to the amount of money listed for good. Yes, we can still behave a bit boorish. Yes, we can still be very similar to scammers.

( Direct Ethereum Address: [0xd9b022cdf82ee7eab9b17c7d85c3ba005e288383](https://etherdonation.com/d?to=0xd9b022cdf82ee7eab9b17c7d85c3ba005e288383) )
