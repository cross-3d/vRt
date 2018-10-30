#!/snap/bin/pwsh-preview

$CFLAGSV="--target-env vulkan1.1 -V -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F32_BVH -DUNIVERSAL_PLATFORM"

$VNDR="universal"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
