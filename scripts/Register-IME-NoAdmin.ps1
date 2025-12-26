# Register-IME-NoAdmin.ps1
# Register the K6 IME without requiring administrator rights

param(
    [string]$DllPath,
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [switch]$RestartCtfmon
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$InstallDir = Join-Path $env:LOCALAPPDATA "K6IME"
$BuildDll = Join-Path $ProjectRoot "build\$Configuration\K6.dll"
$InstalledDll = Join-Path $InstallDir "K6.dll"

if (-not $DllPath) {
    if (Test-Path $InstalledDll) {
        $DllPath = $InstalledDll
    } elseif (Test-Path $BuildDll) {
        $DllPath = $BuildDll
    } else {
        throw "K6.dll not found. Build the project or specify -DllPath."
    }
}

if (-not (Test-Path $DllPath)) {
    throw "DLL not found at $DllPath"
}

$DllPath = (Resolve-Path $DllPath).Path

Write-Host "=== K6 IME User Registration ===" -ForegroundColor Cyan
Write-Host "Using DLL: $DllPath" -ForegroundColor Gray
Write-Host "Registering in current user's registry hive..." -ForegroundColor Yellow

$regArgs = "/s `"$DllPath`""
$process = Start-Process -FilePath "regsvr32.exe" -ArgumentList $regArgs -Wait -PassThru -NoNewWindow

if ($process.ExitCode -ne 0) {
    throw "regsvr32 failed with exit code $($process.ExitCode)"
}

Write-Host "Registration successful." -ForegroundColor Green

if ($RestartCtfmon) {
    Write-Host "Restarting ctfmon.exe to load the new TIP..." -ForegroundColor Yellow
    Stop-Process -Name ctfmon -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 1
    Start-Process "ctfmon.exe"
}

Write-Host "" 
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Settings → Time & Language → Language & region" -ForegroundColor Gray
Write-Host "2. Choose Chinese (Traditional, Hong Kong)" -ForegroundColor Gray
Write-Host "3. Add a keyboard → select 'K6'" -ForegroundColor Gray
Write-Host "4. Switch input with Win+Space" -ForegroundColor Gray
