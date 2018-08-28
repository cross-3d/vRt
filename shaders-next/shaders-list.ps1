# It is helper for compilation shaders to SPIR-V

$INDIR=".\"
$OUTDIR="..\build\shaders\$VNDR\"
$HRDDIR="..\build\intrusive\$VNDR\"

$NTVE="native\"
$VRTX="vertex\"
$RNDR="ray-tracing\"
$HLBV="hlBVH2\"
$RDXI="radix\"
$OUTP="output\"

$CMPPROF="-S comp"
$FRGPROF="-S frag"
$VRTPROF="-S vert"

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

function BuildCompute($Name, $InDir, $OutDir, $AddArg = "", $AltName = $Name) {
    $ARGS = "$CFLAGSV $CMPPROF $InDir$Name -o $OutDir$AltName.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "glslangValidator" -ArgumentList "$ARGS" -PassThru
    $process.WaitForExit()
    $process.Close()
}

function BuildFragment($Name, $InDir, $OutDir, $AddArg = "") {
    $ARGS = "$CFLAGSV $FRGPROF $InDir$Name -o $OutDir$Name.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "glslangValidator" -ArgumentList "$ARGS" -PassThru
    $process.WaitForExit()
    $process.Close()
}

function BuildVertex($Name, $InDir, $OutDir, $AddArg = "") {
    $ARGS = "$CFLAGSV $VRTPROF $InDir$Name -o $OutDir$Name.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "glslangValidator" -ArgumentList "$ARGS" -PassThru
    $process.WaitForExit()
    $process.Close()
}

function BuildAllShaders($Pfx = "") {
    new-item -Name $OUTDIR -itemtype directory       -Force | Out-Null
    new-item -Name $OUTDIR$VRTX -itemtype directory  -Force | Out-Null
    new-item -Name $OUTDIR$RNDR -itemtype directory  -Force | Out-Null
    new-item -Name $OUTDIR$OUTP -itemtype directory  -Force | Out-Null
    
    new-item -Name $HRDDIR$HLBV\\AABB\\ -itemtype directory      -Force | Out-Null
    new-item -Name $HRDDIR$HLBV\\triangle\\ -itemtype directory  -Force | Out-Null
    new-item -Name $HRDDIR$HLBV -itemtype directory              -Force | Out-Null
    new-item -Name $HRDDIR$RDXI -itemtype directory              -Force | Out-Null
    new-item -Name $HRDDIR$NTVE -itemtype directory              -Force | Out-Null

    BuildFragment "render.frag" "$INDIR$OUTP" "$OUTDIR$OUTP"
    BuildVertex   "render.vert" "$INDIR$OUTP" "$OUTDIR$OUTP"

    BuildCompute "closest-hit-shader.comp"  "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "htgen-shader.comp"        "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "closest-hit-shader.comp"  "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "generation-shader.comp"   "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "htgen-shader.comp"        "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "rfgen-shader.comp"        "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "miss-hit-shader.comp"     "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "group-shader.comp"        "$INDIR$RNDR" "$OUTDIR$RNDR"
    BuildCompute "htgrp-shader.comp"        "$INDIR$RNDR" "$OUTDIR$RNDR"

    BuildCompute "vtransformed.comp"        "$INDIR$VRTX" "$OUTDIR$VRTX"

    BuildCompute "vinput.comp"              "$INDIR$NTVE" "$HRDDIR$NTVE"
    BuildCompute "dull.comp"                "$INDIR$NTVE" "$HRDDIR$NTVE"
    BuildCompute "triplet.comp"             "$INDIR$NTVE" "$HRDDIR$NTVE"

    BuildCompute "\\triangle\\bound-calc.comp"  "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "\\triangle\\leaf-gen.comp"    "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "bvh-build-td.comp"            "$INDIR$HLBV" "$HRDDIR$HLBV" "-DFIRST_STEP" "bvh-build-first.comp"
    BuildCompute "bvh-build-td.comp"            "$INDIR$HLBV" "$HRDDIR$HLBV" "" "bvh-build.comp"
    BuildCompute "bvh-fit.comp"                 "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "leaf-link.comp"               "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "shorthand.comp"               "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "traverse-bvh.comp"            "$INDIR$HLBV" "$HRDDIR$HLBV"
    BuildCompute "interpolator.comp"            "$INDIR$HLBV" "$HRDDIR$HLBV"

    BuildCompute "permute.comp"    "$INDIR$RDXI" "$HRDDIR$RDXI"
    BuildCompute "histogram.comp"  "$INDIR$RDXI" "$HRDDIR$RDXI"
    BuildCompute "pfx-work.comp"   "$INDIR$RDXI" "$HRDDIR$RDXI"
    BuildCompute "copyhack.comp"   "$INDIR$RDXI" "$HRDDIR$RDXI"

    Pause #pause for check compile errors
}
