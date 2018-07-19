:: It is helper for compilation shaders to SPIR-V
:: FOR STUPIDS: UNIVERSAL   IS NOT   UWP  ( i.e. != UWP ) 

set PATH=C:\Users\elvir\msvc\glslang\bin;%PATH%

cd %~dp0
  set CFLAGSV= --target-env vulkan1.1 -V130 -d -t --aml --nsf -DUNIVERSAL_PLATFORM -DUSE_F32_BVH -DUSE_MORTON_32
  
set INDIR=.\
set OUTDIR=..\build\shaders\universal\
set HRDDIR=..\include\vRt\HardCodes\universal\
set OUTSHR=..\build\shaders\

call shaders-list.cmd

pause
