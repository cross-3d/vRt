#!/usr/bin/pwsh-preview

 $CFLAGSV="--target-env vulkan1.1 --client vulkan100 -d -t --aml --nsf -DUSE_MORTON_32 -DUSE_F32_BVH -DAMD_PLATFORM"

$VNDR="amd"
. "./shaders-list.ps1"

BuildAllShaders ""
