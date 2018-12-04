#!/snap/bin/pwsh-preview

$CFLAGSV="--target-env spirv1.3 -V -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F32_BVH -DENABLE_SCALAR_BLOCK_LAYOUT -DNVIDIA_PLATFORM"

$VNDR="nvidia"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
