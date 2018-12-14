#!/snap/bin/pwsh-preview

 $CFLAGSV="--target-env spirv1.3 -V -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F32_BVH -DAMD_PLATFORM -DENABLE_VEGA_INSTRUCTION_SET -DENABLE_RADV"
#$CFLAGSV="--target-env spirv1.3 -V -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F16_BVH -DAMD_PLATFORM -DENABLE_VEGA_INSTRUCTION_SET -DENABLE_RADV"

$VNDR="radv"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
