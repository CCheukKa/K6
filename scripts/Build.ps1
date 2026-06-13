# Build.ps1 - Build the Chinese IME
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [string]$Generator = "",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

function Invoke-CMakeConfigure {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SelectedGenerator
    )

    Write-Host "Using generator: $SelectedGenerator" -ForegroundColor DarkCyan
    cmake -S $ProjectRoot -B $BuildDir -G $SelectedGenerator -A x64
    return ($LASTEXITCODE -eq 0)
}

function Get-VisualStudioGenerator {
    $VsWherePath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"

    if (Test-Path $VsWherePath) {
        $VsWhereOutput = & $VsWherePath -latest -products * -requires Microsoft.Component.MSBuild -format json
        if ($LASTEXITCODE -eq 0 -and $VsWhereOutput) {
            $Instances = $VsWhereOutput | ConvertFrom-Json
            if ($Instances) {
                $LatestInstance = $Instances | Select-Object -First 1
                switch ($LatestInstance.catalog.productLineVersion) {
                    "2026" { return "Visual Studio 18 2026" }
                    "2022" { return "Visual Studio 17 2022" }
                    "2019" { return "Visual Studio 16 2019" }
                }
            }
        }
    }

    return @(
        "Visual Studio 18 2026",
        "Visual Studio 17 2022",
        "Visual Studio 16 2019"
    )
}

Write-Host "=== Building ChineseIME ($Configuration) ===" -ForegroundColor Cyan

# Clean build if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Configure if needed
if (-not (Test-Path $BuildDir)) {
    Write-Host "Configuring CMake..." -ForegroundColor Yellow

    $GeneratorCandidates = if ($Generator) { @($Generator) } else { Get-VisualStudioGenerator }

    foreach ($Candidate in $GeneratorCandidates) {
        if (Invoke-CMakeConfigure -SelectedGenerator $Candidate) {
            break
        }

        if (Test-Path $BuildDir) {
            Remove-Item -Recurse -Force $BuildDir
        }
    }

    if (-not (Test-Path $BuildDir)) {
        Write-Host "CMake configuration failed!" -ForegroundColor Red
        exit 1
    }
}

# Build
Write-Host "Building..." -ForegroundColor Yellow
cmake --build $BuildDir --config $Configuration

if ($LASTEXITCODE -eq 0) {
    $DllPath = Join-Path $BuildDir "output\K6.dll"
    Write-Host "`nBuild successful!" -ForegroundColor Green
    Write-Host "Output: $DllPath" -ForegroundColor Gray
} else {
    Write-Host "`nBuild failed!" -ForegroundColor Red
    exit 1
}
