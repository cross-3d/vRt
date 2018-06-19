:: It is helper for compilation shaders to SPIR-V

set VRTX=vertex\
set RNDR=rayTracing\
set HLBV=hlBVH2\
set RDXI=qRadix\
set OUTP=output\

set CMPPROF=-fshader-stage=compute
set FRGPROF=-fshader-stage=fragment
set VRTPROF=-fshader-stage=vertex
set GMTPROF=-fshader-stage=geometry

mkdir %OUTDIR%
mkdir %OUTDIR%%VRTX%
mkdir %OUTDIR%%RNDR%
mkdir %OUTDIR%%HLBV%
mkdir %OUTDIR%%RDXI%
mkdir %OUTDIR%%OUTP%
mkdir %OUTDIR%%GENG%
mkdir %HRDDIR%%HLBV%
mkdir %HRDDIR%%RDXI%

start /b /wait glslangValidator %CFLAGSV% %INDIR%%OUTP%render.frag        -o %OUTDIR%%OUTP%render.frag.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%OUTP%render.vert        -o %OUTDIR%%OUTP%render.vert.spv

start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%closest-hit-shader.comp -o %OUTDIR%%RNDR%closest-hit-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%generation-shader.comp  -o %OUTDIR%%RNDR%generation-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%miss-hit-shader.comp    -o %OUTDIR%%RNDR%miss-hit-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%resolve-shader.comp     -o %OUTDIR%%RNDR%resolve-shader.comp.spv

start /b /wait glslangValidator %CFLAGSV% %INDIR%%VRTX%vinput.comp        -o %OUTDIR%%VRTX%vinput.comp.spv

start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%bound-calc.comp   -x -o %HRDDIR%%HLBV%bound-calc.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%bvh-build.comp    -x -o %HRDDIR%%HLBV%bvh-build.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%bvh-fit.comp      -x -o %HRDDIR%%HLBV%bvh-fit.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%leaf-gen.comp     -x -o %HRDDIR%%HLBV%leaf-gen.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%leaf-link.comp    -x -o %HRDDIR%%HLBV%leaf-link.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%shorthand.comp    -x -o %HRDDIR%%HLBV%shorthand.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%traverse-bvh.comp -x -o %HRDDIR%%HLBV%traverse-bvh.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%interpolator.comp -x -o %HRDDIR%%HLBV%interpolator.comp.inl

start /b /wait glslangValidator %CFLAGSV% %INDIR%%RDXI%permute.comp      -x -o %HRDDIR%%RDXI%permute.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RDXI%histogram.comp    -x -o %HRDDIR%%RDXI%histogram.comp.inl
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RDXI%pfx-work.comp     -x -o %HRDDIR%%RDXI%pfx-work.comp.inl

:: --ccp not supported by that renderer 

set FIXFLAGS = ^
--skip-validation ^
--strip-debug ^
--workaround-1209 ^
--replace-invalid-opcode ^
--simplify-instructions ^
--cfg-cleanup ^
-Os

set OPTFLAGS= ^
--skip-validation ^
--private-to-local ^
--ccp ^
--unify-const ^
--flatten-decorations ^
--fold-spec-const-op-composite ^
--strip-debug ^
--freeze-spec-const ^
--cfg-cleanup ^
--merge-blocks ^
--merge-return ^
--strength-reduction ^
--inline-entry-points-exhaustive ^
--convert-local-access-chains ^
--eliminate-dead-code-aggressive ^
--eliminate-dead-branches ^
--eliminate-dead-const ^
--eliminate-dead-variables ^
--eliminate-dead-functions ^
--eliminate-local-single-block ^
--eliminate-local-single-store ^
--eliminate-local-multi-store ^
--eliminate-common-uniform ^
--eliminate-insert-extract ^
--scalar-replacement ^
--relax-struct-store ^
--redundancy-elimination ^
--remove-duplicates ^
--private-to-local ^
--local-redundancy-elimination ^
--cfg-cleanup ^
--workaround-1209 ^
--replace-invalid-opcode ^
--if-conversion ^
--scalar-replacement ^
--simplify-instructions

call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%interpolator.comp.spv    -o %OUTDIR%%HLBV%interpolator.comp.spv
call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%traverse-bvh.comp.spv    -o %OUTDIR%%HLBV%traverse-bvh.comp.spv
call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%bvh-build.comp.spv       -o %OUTDIR%%HLBV%bvh-build.comp.spv
call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%bound-calc.comp.spv      -o %OUTDIR%%HLBV%bound-calc.comp.spv
call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%leaf-gen.comp.spv        -o %OUTDIR%%HLBV%leaf-gen.comp.spv
