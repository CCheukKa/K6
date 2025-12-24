# Build.ps1 - Build the Chinese IME
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

Write-Host "=== Building ChineseIME ($Configuration) ===" -ForegroundColor Cyan

# Clean build if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Configure if needed
if (-not (Test-Path $BuildDir)) {
    Write-Host "Configuring CMake..." -ForegroundColor Yellow
    cmake -S $ProjectRoot -B $BuildDir -G "Visual Studio 17 2022" -A x64
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configuration failed!" -ForegroundColor Red
        exit 1
    }
}

# Build
Write-Host "Building..." -ForegroundColor Yellow
cmake --build $BuildDir --config $Configuration

if ($LASTEXITCODE -eq 0) {
    $DllPath = Join-Path $BuildDir "$Configuration\ChineseIMEHKExample.dll"
    Write-Host "`nBuild successful!" -ForegroundColor Green
    Write-Host "Output: $DllPath" -ForegroundColor Gray
} else {
    Write-Host "`nBuild failed!" -ForegroundColor Red
    exit 1
}
