:: It is helper for compilation shaders to SPIR-V

set PATH=C:\Users\elvir\msvc\glslang\bin;%PATH%

cd %~dp0
  set CFLAGSV= --client vulkan100 --target-env vulkan1.1 -d --aml -DUNIVERSAL_PLATFORM -DUSE_F32_BVH 
  
set INDIR=.\
set OUTDIR=..\build\shaders\universal\
set OUTSHR=..\build\shaders\

call shaders-list.cmd

pause
