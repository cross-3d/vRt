#!/snap/bin/pwsh-preview

 $CFLAGSV="--target-env vulkan1.1 -V -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F32_BVH -DAMD_PLATFORM -DENABLE_VEGA_INSTRUCTION_SET"
#$CFLAGSV="--target-env vulkan1.1 -V -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F16_BVH -DAMD_PLATFORM -DENABLE_VEGA_INSTRUCTION_SET"

$VNDR="amd"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
