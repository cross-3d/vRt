:: It is helper for compilation shaders to SPIR-V

set PATH=C:\Users\elvir\msvc\glslang\bin;%PATH%

cd %~dp0
  set CFLAGSV= --target-env vulkan1.1 -d --aml -DAMD_PLATFORM -DENABLE_AMD_INSTRUCTION_SET -DUSE_F32_BVH 

set INDIR=.\
set OUTDIR=..\build\shaders\amd\
set HRDDIR=..\include\vRt\HardCodes\amd\
set OUTSHR=..\build\shaders\

call shaders-list.cmd

pause
