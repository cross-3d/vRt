:: It is helper for compilation shaders to SPIR-V


set PATH=C:\Users\elvir\msvc\glslang\bin;%PATH%

cd %~dp0
set CFLAGSV= --target-env vulkan1.1 --client vulkan100 -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F32_BVH -DINTEL_PLATFORM -DPLAIN_BINDLESS_TEXTURE_FETCH 

set VNDR=intel
call shaders-list.cmd
pause
