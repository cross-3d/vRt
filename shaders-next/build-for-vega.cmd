:: It is helper for compilation shaders to SPIR-V

set PATH=C:\Users\elvir\msvc\glslang\bin;%PATH%

cd %~dp0
::set CFLAGSV= --target-env vulkan1.1 --client vulkan100 -d -t --aml --nsf -DAMD_PLATFORM -DUSE_F32_BVH -DUSE_MORTON_32 -DENABLE_AMD_INSTRUCTION_SET
  set CFLAGSV= --target-env vulkan1.1 --client vulkan100 -d -t --aml --nsf -DAMD_PLATFORM -DAMD_F16_BVH -DUSE_MORTON_32 -DENABLE_AMD_INSTRUCTION_SET

set VNDR=amd
call shaders-list.cmd
pause
