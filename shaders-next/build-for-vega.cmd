:: It is helper for compilation shaders to SPIR-V


set PATH=C:\Users\elvir\msvc\glslang\bin;%PATH%

cd %~dp0
set CFLAGSV= --target-env vulkan1.1 --client vulkan100 -d -t --aml --nsf -DUSE_MORTON_32 -DAMD_F16_BVH -DAMD_PLATFORM -DENABLE_AMD_INSTRUCTION_SET

set VNDR=amd
call shaders-list.cmd
pause
