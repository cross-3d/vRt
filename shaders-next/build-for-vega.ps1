#!/usr/bin/pwsh-preview

$CFLAGSV="--target-env vulkan1.1 --client vulkan100 -d -t --aml --nsf -DUSE_MORTON_32 -DAMD_F16_BVH -DAMD_PLATFORM -DENABLE_AMD_INSTRUCTION_SET"

$VNDR="amd"
. "./shaders-list.ps1"

BuildAllShaders ""
