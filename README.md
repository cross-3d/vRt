# vRt (Vulkan API real-time ray tracing library)

<a href="https://gitter.im/world8th/vRt?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge" target="_blank" title="Gitter"><img src="https://badges.gitter.im/world8th/vRt.svg" alt="Donate ETH" height="20px"/></a>
<a href="https://etherdonation.com/d?to=0x06d105B9508c359e5dD56631ea572e5fEF5F41ae" target="_blank" title="Donate ETH"><img src="https://etherdonation.com/i/btn/donate-btn.png" alt="Donate ETH" height="20px"/></a>

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
- Roundly 100Mrays/s in top GPU's (tested RX Vega 64, in Sponza scene, fully dynamic mode)

## Repository mirrors

- https://github.com/world8th/vRt
- https://gitlab.com/world8th/vRt

## Requirements

- Vulkan API 1.1 Support
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- C++17 capable compiler
- GLSL to SPIR-V 1.3 compiler
- GLM for test application run

## Compatibility Confirmed

- RX Vega 64
- GTX 1070

## Uncomfirmed

- RTX 2080, 2080 Ti (non-RTX mode only)
- GTX 1070 Ti, 1080 Ti (Pascal), 980, 980 Ti (Maxwell)
- RX Vega 56 and Raven Ridge
- RX 580, 480 (optimizations ready only for RX Vega 64 and that series)

## Current API

You can see here current API of library:

- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/API.inl
- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/Structures.inl
- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/StructuresDef.inl
- https://github.com/world8th/vRt/blob/master/include/vRt/Parts/Enums.inl

## Roadmaps

### 2018 Q4 ("Lithium")

- Coming to alpha stage
- Better ray-tracing examples
- Advanced ray-tracing API

### Beyond

- First implementations in bigger framework and engines
- Make custom GLSL/HLSL compiler for unified compatibility
- Support of Nvidia RTX, PowerVR Wizard Ray Tracing, Intel Larrabee 2019

## Donation

For development of RTX back-end, need around 700$ for purchase NVidia RTX 2080 series card... You can donate for us.

In order to avoid charges of fraud, I will say directly ... Yes, I can not guarantee you a refund, I do not guarantee that this product will be useful for you, I do not guarantee any benefits to you, I can not guarantee that I will meet your expectations. By sending us a donation, you take your risk and say goodbye to the amount of money listed for good. Yes, we can still behave a bit boorish. Yes, we can still be very similar to scammers.

( Direct Ethereum Address: [0x06d105B9508c359e5dD56631ea572e5fEF5F41ae](https://etherdonation.com/d?to=0x06d105B9508c359e5dD56631ea572e5fEF5F41ae) )
