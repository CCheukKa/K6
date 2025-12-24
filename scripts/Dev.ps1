# Dev.ps1 - Development helper: Unregister, Build, Register in one step
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$ScriptDir = $PSScriptRoot

Write-Host "=== Development Build Cycle ===" -ForegroundColor Magenta
Write-Host ""

# Step 1: Unregister and stop ctfmon
Write-Host "[1/3] Unregistering existing IME..." -ForegroundColor Cyan
& "$ScriptDir\Unregister.ps1" -Configuration $Configuration -StopCtfmon

Write-Host ""

# Step 2: Build
Write-Host "[2/3] Building..." -ForegroundColor Cyan
if ($Clean) {
    & "$ScriptDir\Build.ps1" -Configuration $Configuration -Clean
} else {
    & "$ScriptDir\Build.ps1" -Configuration $Configuration
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed, stopping." -ForegroundColor Red
    exit 1
}

Write-Host ""

# Step 3: Register
Write-Host "[3/3] Registering new build..." -ForegroundColor Cyan
& "$ScriptDir\Register.ps1" -Configuration $Configuration

Write-Host ""
Write-Host "=== Done! ===" -ForegroundColor Green
Write-Host "Press Win+Space to switch to the IME" -ForegroundColor Gray
