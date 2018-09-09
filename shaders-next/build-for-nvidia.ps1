#!/usr/bin/pwsh-preview

$CFLAGSV="--target-env vulkan1.1 --client vulkan100 -d -t --aml --nsf -DUSE_F32_BVH -DNVIDIA_PLATFORM"

$VNDR="nvidia"
. "./shaders-list.ps1"

BuildAllShaders ""
