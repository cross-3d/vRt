:: It is helper for compilation shaders to SPIR-V

set INDIR=.\
set OUTDIR=..\build\shaders\%VNDR%\
::set HRDDIR=%OUTDIR%
::set HRDDIR=..\include\vRt\HardCodes\%VNDR%\
set HRDDIR=..\build\intrusive\%VNDR%\
set OUTSHR=..\build\shaders\



set NTVE=native\
set VRTX=vertex\
set RNDR=ray-tracing\
set HLBV=hlBVH2\
set RDXI=radix\
set OUTP=output\

set CMPPROF=-fshader-stage=compute
set FRGPROF=-fshader-stage=fragment
set VRTPROF=-fshader-stage=vertex
set GMTPROF=-fshader-stage=geometry
::set HEXT=.inl
set HEXT=.spv

mkdir %OUTDIR%
mkdir %OUTDIR%%VRTX%
mkdir %OUTDIR%%RNDR%
mkdir %OUTDIR%%OUTP%

mkdir %HRDDIR%%HLBV%\\AABB\\
mkdir %HRDDIR%%HLBV%\\triangle\\
mkdir %HRDDIR%%HLBV%
mkdir %HRDDIR%%RDXI%
mkdir %HRDDIR%%NTVE%



start /b /wait glslangValidator %CFLAGSV% %INDIR%%OUTP%render.frag        -o %OUTDIR%%OUTP%render.frag.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%OUTP%render.vert        -o %OUTDIR%%OUTP%render.vert.spv

start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%closest-hit-shader.comp -o %OUTDIR%%RNDR%closest-hit-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%generation-shader.comp  -o %OUTDIR%%RNDR%generation-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%miss-hit-shader.comp    -o %OUTDIR%%RNDR%miss-hit-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RNDR%group-shader.comp       -o %OUTDIR%%RNDR%group-shader.comp.spv
start /b /wait glslangValidator %CFLAGSV% %INDIR%%VRTX%vtransformed.comp       -o %OUTDIR%%VRTX%vtransformed.comp.spv

start /b /wait glslangValidator %CFLAGSV% %INDIR%%NTVE%vinput.comp        -o %HRDDIR%%NTVE%vinput.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%NTVE%dull.comp          -o %HRDDIR%%NTVE%dull.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%NTVE%triplet.comp       -o %HRDDIR%%NTVE%triplet.comp%HEXT%

start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%\\triangle\\bound-calc.comp    -o %HRDDIR%%HLBV%\\triangle\\bound-calc.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%\\triangle\\leaf-gen.comp      -o %HRDDIR%%HLBV%\\triangle\\leaf-gen.comp%HEXT%

start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%bvh-build-td.comp     -o %HRDDIR%%HLBV%bvh-build-first.comp%HEXT% -DFIRST_STEP
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%bvh-build-td.comp     -o %HRDDIR%%HLBV%bvh-build.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%bvh-fit.comp       -o %HRDDIR%%HLBV%bvh-fit.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%leaf-link.comp     -o %HRDDIR%%HLBV%leaf-link.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%shorthand.comp     -o %HRDDIR%%HLBV%shorthand.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%traverse-bvh.comp  -o %HRDDIR%%HLBV%traverse-bvh.comp%HEXT% 
start /b /wait glslangValidator %CFLAGSV% %INDIR%%HLBV%interpolator.comp  -o %HRDDIR%%HLBV%interpolator.comp%HEXT%

start /b /wait glslangValidator %CFLAGSV% %INDIR%%RDXI%permute.comp       -o %HRDDIR%%RDXI%permute.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RDXI%histogram.comp     -o %HRDDIR%%RDXI%histogram.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RDXI%pfx-work.comp      -o %HRDDIR%%RDXI%pfx-work.comp%HEXT%
start /b /wait glslangValidator %CFLAGSV% %INDIR%%RDXI%copyhack.comp      -o %HRDDIR%%RDXI%copyhack.comp%HEXT%

set FIXFLAGS = -Os 
set OPTFLAGS = -O 


call spirv-opt %OPTFLAGS% %HRDDIR%%HLBV%interpolator.comp.spv    -o %HRDDIR%%HLBV%interpolator.comp.spv
call spirv-opt %OPTFLAGS% %HRDDIR%%HLBV%traverse-bvh.comp.spv    -o %HRDDIR%%HLBV%traverse-bvh.comp.spv
call spirv-opt %OPTFLAGS% %HRDDIR%%HLBV%bvh-build.comp.spv       -o %HRDDIR%%HLBV%bvh-build.comp.spv
call spirv-opt %OPTFLAGS% %HRDDIR%%HLBV%bvh-build-first.comp.spv -o %HRDDIR%%HLBV%bvh-build-first.comp.spv
call spirv-opt %OPTFLAGS% %HRDDIR%%HLBV%\\triangle\\bound-calc.comp.spv      -o %HRDDIR%%HLBV%\\triangle\\bound-calc.comp.spv
call spirv-opt %OPTFLAGS% %HRDDIR%%HLBV%\\triangle\\leaf-gen.comp.spv        -o %HRDDIR%%HLBV%\\triangle\\leaf-gen.comp.spv
