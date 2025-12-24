# Unregister.ps1 - Unregister the Chinese IME
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [switch]$StopCtfmon
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$DllPath = Join-Path $ProjectRoot "build\$Configuration\ChineseIMEHKExample.dll"

Write-Host "=== Unregistering ChineseIME ===" -ForegroundColor Cyan

if (-not (Test-Path $DllPath)) {
    Write-Host "DLL not found: $DllPath" -ForegroundColor Yellow
    Write-Host "Nothing to unregister." -ForegroundColor Gray
    exit 0
}

Write-Host "Unregistering: $DllPath" -ForegroundColor Yellow

# Check if running as admin
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if ($isAdmin) {
    regsvr32 /u /s $DllPath
    Write-Host "Unregistration successful!" -ForegroundColor Green
    
    if ($StopCtfmon) {
        Write-Host "Stopping ctfmon to release DLL..." -ForegroundColor Yellow
        Stop-Process -Name ctfmon -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 1
        Write-Host "ctfmon stopped. It will restart automatically when needed." -ForegroundColor Gray
    }
} else {
    Write-Host "Requesting administrator privileges..." -ForegroundColor Yellow
    Start-Process regsvr32 -ArgumentList "/u `"$DllPath`"" -Verb RunAs -Wait
    Write-Host "Unregistration complete!" -ForegroundColor Green
    
    if ($StopCtfmon) {
        Write-Host "Note: Run with admin privileges to stop ctfmon" -ForegroundColor Yellow
    }
}
