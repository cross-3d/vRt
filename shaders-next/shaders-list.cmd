:: It is helper for compilation shaders to SPIR-V

set NTVE=native\
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
mkdir %HRDDIR%%NTVE%


start /b /wait glslangValidator %CFLAGSV% %INDIR%%OUTP%render.frag        -o %OUTDIR%%OUTP%render.frag.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%OUTP%render.vert        -o %OUTDIR%%OUTP%render.vert.spv

start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%closest-hit-shader.comp -o %OUTDIR%%RNDR%closest-hit-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%generation-shader.comp  -o %OUTDIR%%RNDR%generation-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%miss-hit-shader.comp    -o %OUTDIR%%RNDR%miss-hit-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%resolve-shader.comp     -o %OUTDIR%%RNDR%resolve-shader.comp.spv

start /b /wait glslangValidator %CFLAGSV% %INDIR%%NTVE%vinput.comp       -x -o %HRDDIR%%NTVE%vinput.comp.inl

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
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RDXI%copyhack.comp     -x -o %HRDDIR%%RDXI%copyhack.comp.inl

:: --ccp not supported by that renderer 

set FIXFLAGS = -Os 
set OPTFLAGS = -O 

::call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%interpolator.comp.spv    -o %OUTDIR%%HLBV%interpolator.comp.spv
::call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%traverse-bvh.comp.spv    -o %OUTDIR%%HLBV%traverse-bvh.comp.spv
::call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%bvh-build.comp.spv       -o %OUTDIR%%HLBV%bvh-build.comp.spv
::call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%bound-calc.comp.spv      -o %OUTDIR%%HLBV%bound-calc.comp.spv
::call spirv-opt %FIXFLAGS% %OUTDIR%%HLBV%leaf-gen.comp.spv        -o %OUTDIR%%HLBV%leaf-gen.comp.spv
