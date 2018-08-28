
$CFLAGSV="--target-env vulkan1.1 --client vulkan100 -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F32_BVH -DNVIDIA_PLATFORM"

$VNDR="nvidia"
. "./shaders-list.ps1"

BuildAllShaders ""
