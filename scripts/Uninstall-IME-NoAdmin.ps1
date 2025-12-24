# Uninstall-IME-NoAdmin.ps1
# Remove K6 IME installation from user profile

$ErrorActionPreference = "Stop"

$InstallDir = Join-Path $env:LOCALAPPDATA "K6IME"
$ClsidGuid = "{B6D63B6F-4584-4652-A02B-8D30AFAA61B1}"
$ClassesPath = "HKCU:\Software\Classes"

Write-Host "=== K6 IME Uninstaller ===" -ForegroundColor Cyan
Write-Host ""

# Remove registry entries
Write-Host "Removing registry entries..." -ForegroundColor Yellow
Remove-Item -Path "$ClassesPath\CLSID\$ClsidGuid" -Recurse -Force -ErrorAction SilentlyContinue

# Remove installation directory
Write-Host "Removing installation directory..." -ForegroundColor Yellow
if (Test-Path $InstallDir) {
    Remove-Item -Path $InstallDir -Recurse -Force
}

Write-Host "Uninstallation complete!" -ForegroundColor Green
Write-Host ""
Write-Host "To remove from Windows input methods:" -ForegroundColor Cyan
Write-Host "1. Go to Settings → Time & Language → Language & region" -ForegroundColor Gray
Write-Host "2. Find K6 in your Chinese (Traditional, Hong Kong) keyboard list" -ForegroundColor Gray
Write-Host "3. Select it and click 'Remove'" -ForegroundColor Gray
