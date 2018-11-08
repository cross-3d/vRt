#!/snap/bin/pwsh-preview
# NOT SUPPORTED OFFICIALLY!

$CFLAGSV="--target-env spirv1.3 -V -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F32_BVH -DINTEL_PLATFORM"

$VNDR="intel"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
