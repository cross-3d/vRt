#!/usr/bin/pwsh-preview

# It is helper for compilation shaders to SPIR-V

$INDIR="./"
$OUTDIR="../build/shaders/$VNDR/"
$HRDDIR="../build/intrusive/$VNDR/"

$NTVE="native/"
$VRTX="vertex/"
$RNDR="ray-tracing/"
$HLBV="hlBVH2/"
$RDXI="radix/"
$OUTP="output/"

$CMPPROF="-S comp"
$FRGPROF="-S frag"
$VRTPROF="-S vert"
$OPTFLAGS="-O"

function Pause ($Message = "Press any key to continue . . . ") {
    if ((Test-Path variable:psISE) -and $psISE) {
        $Shell = New-Object -ComObject "WScript.Shell"
        $Button = $Shell.Popup("Click OK to continue.", 0, "Script Paused", 0)
    }
    else {     
        Write-Host -NoNewline $Message
        [void][System.Console]::ReadKey($true)
        Write-Host
    }
}

function Optimize($Name, $Dir = "", $AddArg = "") {
    $ARGS = "$OPTFLAGS $Dir$Name.spv -o $Dir$Name.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "spirv-opt" -ArgumentList "$ARGS" -PassThru
    $process.PriorityClass = 'BelowNormal'
    $process.WaitForExit()
    $process.Close()
}

function BuildCompute($Name, $InDir = "", $OutDir = "", $AddArg = "", $AltName = $Name) {
    $ARGS = "$CFLAGSV $CMPPROF $InDir$Name -o $OutDir$AltName.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "glslangValidator" -ArgumentList "$ARGS" -PassThru
    $process.PriorityClass = 'BelowNormal'
    $process.WaitForExit()
    $process.Close()
}

function BuildFragment($Name, $InDir = "", $OutDir = "", $AddArg = "") {
    $ARGS = "$CFLAGSV $FRGPROF $InDir$Name -o $OutDir$Name.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "glslangValidator" -ArgumentList "$ARGS" -PassThru
    $process.PriorityClass = 'BelowNormal'
    $process.WaitForExit()
    $process.Close()
}

function BuildVertex($Name, $InDir = "", $OutDir = "", $AddArg = "") {
    $ARGS = "$CFLAGSV $VRTPROF $InDir$Name -o $OutDir$Name.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "glslangValidator" -ArgumentList "$ARGS" -PassThru
    $process.PriorityClass = 'BelowNormal'
    $process.WaitForExit()
    $process.Close()
}

function OptimizeMainline($Pfx = "") {
    # optimize accelerator structure (hlBVH2)
    #Optimize "interpolator.comp" "$HRDDIR$HLBV"
    #Optimize "traverse-bvh.comp" "$HRDDIR$HLBV" 
    Optimize "bvh-build-first.comp" "$HRDDIR$HLBV" 
    Optimize "bvh-build.comp" "$HRDDIR$HLBV" 
    Optimize "bvh-fit.comp" "$HRDDIR$HLBV" 
    Optimize "shorthand.comp" "$HRDDIR$HLBV" 
    Optimize "leaf-link.comp" "$HRDDIR$HLBV" 
    
    Optimize "/triangle/bound-calc.comp" "$HRDDIR$HLBV" 
    Optimize "/triangle/leaf-gen.comp" "$HRDDIR$HLBV" 
    Optimize "/triangle/box-calc.comp" "$HRDDIR$HLBV" 
    
    # optimize vertex assemblers
    #Optimize "vinput.comp"       "$HRDDIR$NTVE" # native
    #Optimize "vtransformed.comp" "$OUTDIR$VRTX" 

    # optimize radix sort
    Optimize "permute.comp"   "$HRDDIR$RDXI"
    Optimize "histogram.comp" "$HRDDIR$RDXI"
    Optimize "pfx-work.comp"  "$HRDDIR$RDXI"
    Optimize "copyhack.comp"  "$HRDDIR$RDXI"
}

function BuildAllShaders($Pfx = "") {
    [System.Threading.Thread]::CurrentThread.Priority = 'BelowNormal'

    new-item -Name $OUTDIR -itemtype directory       -Force | Out-Null
    new-item -Name $OUTDIR$VRTX -itemtype directory  -Force | Out-Null
    new-item -Name $OUTDIR$RNDR -itemtype directory  -Force | Out-Null
    new-item -Name $OUTDIR$OUTP -itemtype directory  -Force | Out-Null
    
    new-item -Name $HRDDIR$HLBV//AABB// -itemtype directory      -Force | Out-Null
    new-item -Name $HRDDIR$HLBV//triangle// -itemtype directory  -Force | Out-Null
    new-item -Name $HRDDIR$HLBV -itemtype directory              -Force | Out-Null
    new-item -Name $HRDDIR$RDXI -itemtype directory              -Force | Out-Null
    new-item -Name $HRDDIR$NTVE -itemtype directory              -Force | Out-Null

    # output shader
    BuildFragment "render.frag" "$INDIR$OUTP" "$OUTDIR$OUTP"
    BuildVertex   "render.vert" "$INDIR$OUTP" "$OUTDIR$OUTP"

    # ray tracing shaders
    BuildCompute "closest-hit-shader.comp"  "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "generation-shader.comp"   "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "rfgen-shader.comp"        "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "miss-hit-shader.comp"     "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "group-shader.comp"        "$INDIR$RNDR" "$OUTDIR$RNDR"

    # vertex assemblers
    BuildCompute "vattributes.comp"         "$INDIR$VRTX" "$OUTDIR$VRTX"
    BuildCompute "vinput.comp"              "$INDIR$NTVE" "$HRDDIR$NTVE"

    #BuildCompute "dull.comp"                "$INDIR$NTVE" "$HRDDIR$NTVE"
    #BuildCompute "triplet.comp"             "$INDIR$NTVE" "$HRDDIR$NTVE"

    # accelerator structure (hlBVH2)
    BuildCompute "/triangle/bound-calc.comp"    "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "/triangle/leaf-gen.comp"      "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "/triangle/box-calc.comp"      "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "bvh-build-td.comp"            "$INDIR$HLBV" "$HRDDIR$HLBV" "-DFIRST_STEP" "bvh-build-first.comp"
    BuildCompute "bvh-build-td.comp"            "$INDIR$HLBV" "$HRDDIR$HLBV" "" "bvh-build.comp"
    BuildCompute "bvh-fit.comp"                 "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "leaf-link.comp"               "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "shorthand.comp"               "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "traverse-bvh.comp"            "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "interpolator.comp"            "$INDIR$HLBV" "$HRDDIR$HLBV"

    # radix sort
    BuildCompute "permute.comp"    "$INDIR$RDXI" "$HRDDIR$RDXI"
    BuildCompute "histogram.comp"  "$INDIR$RDXI" "$HRDDIR$RDXI"
    BuildCompute "pfx-work.comp"   "$INDIR$RDXI" "$HRDDIR$RDXI"
    BuildCompute "copyhack.comp"   "$INDIR$RDXI" "$HRDDIR$RDXI"

    # optimize built shaders
    OptimizeMainline

    [System.Threading.Thread]::CurrentThread.Priority = 'Highest'
    
    #pause for check compile errors
    Pause 
}
