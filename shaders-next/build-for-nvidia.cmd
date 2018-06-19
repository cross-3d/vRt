:: It is helper for compilation shaders to SPIR-V

set PATH=C:\Users\elvir\msvc\glslang\bin;%PATH%

cd %~dp0
  set CFLAGSV= --client vulkan100 --target-env vulkan1.1 -d --aml -DNVIDIA_PLATFORM -DUSE_F32_BVH -DPLAIN_BINDLESS_TEXTURE_FETCH
  
set INDIR=.\
set OUTDIR=..\build\shaders\nvidia\
set HRDDIR=..\include\vRt\HardCodes\nvidia\
set OUTSHR=..\build\shaders\

call shaders-list.cmd

pause
