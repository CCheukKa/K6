# Register.ps1 - Register the Chinese IME
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$DllPath = Join-Path $ProjectRoot "build\$Configuration\K6.dll"

Write-Host "=== Registering ChineseIME ===" -ForegroundColor Cyan

if (-not (Test-Path $DllPath)) {
    Write-Host "DLL not found: $DllPath" -ForegroundColor Red
    Write-Host "Run Build.ps1 first!" -ForegroundColor Yellow
    exit 1
}

Write-Host "Registering: $DllPath" -ForegroundColor Yellow

# Check if running as admin
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if ($isAdmin) {
    regsvr32 /s $DllPath
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Registration successful!" -ForegroundColor Green
    } else {
        Write-Host "Registration failed!" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "Requesting administrator privileges..." -ForegroundColor Yellow
    Start-Process regsvr32 -ArgumentList "`"$DllPath`"" -Verb RunAs -Wait
    Write-Host "Registration complete!" -ForegroundColor Green
}

Write-Host "`nTo use the IME:" -ForegroundColor Cyan
Write-Host "1. Open Settings > Time & Language > Language & Region" -ForegroundColor Gray
Write-Host "2. Add/Select Chinese (Traditional) language" -ForegroundColor Gray
Write-Host "3. Add keyboard and select the IME" -ForegroundColor Gray
Write-Host "4. Press Win+Space to switch input methods" -ForegroundColor Gray
