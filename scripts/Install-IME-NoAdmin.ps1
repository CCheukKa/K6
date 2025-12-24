# Install-IME-NoAdmin.ps1
# Non-privileged IME installer using registry hives and user-specific registration
# This script installs the K6 IME into the current user's profile without requiring admin rights

param(
    [Parameter(Mandatory = $false)]
    [string]$DllPath,
    
    [Parameter(Mandatory = $false)]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

# Get paths
$ProjectRoot = if ($DllPath) {
    Split-Path -Parent (Split-Path -Parent $DllPath)
} else {
    Split-Path -Parent $PSScriptRoot  # $PSScriptRoot is scripts folder, parent is K6 folder
}

$BuildDir = Join-Path $ProjectRoot "build"
$DllPath = if ($DllPath) { $DllPath } else { Join-Path $BuildDir "$Configuration\K6.dll" }
$DataDir = Join-Path $ProjectRoot "data"
$InstallDir = Join-Path $env:LOCALAPPDATA "K6IME"

Write-Host "=== K6 IME Non-Admin Installer ===" -ForegroundColor Cyan
Write-Host "Project Root: $ProjectRoot" -ForegroundColor Gray
Write-Host "Install Dir: $InstallDir" -ForegroundColor Gray
Write-Host ""

# Check if DLL exists
if (-not (Test-Path $DllPath)) {
    Write-Host "ERROR: DLL not found at $DllPath" -ForegroundColor Red
    Write-Host "Make sure to build first using: .\Build.ps1" -ForegroundColor Yellow
    exit 1
}

# Create installation directory
Write-Host "Creating installation directory..." -ForegroundColor Yellow
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}

# Copy DLL
Write-Host "Copying DLL to $InstallDir..." -ForegroundColor Yellow
Copy-Item -Path $DllPath -Destination (Join-Path $InstallDir "K6.dll") -Force

# Copy data files
Write-Host "Copying data files..." -ForegroundColor Yellow
if (Test-Path $DataDir) {
    Copy-Item -Path (Join-Path $DataDir "*") -Destination $InstallDir -Recurse -Force
}

# Register in user-specific HKCU (no admin required)
Write-Host "Registering in HKCU..." -ForegroundColor Yellow

$DllFullPath = Join-Path $InstallDir "K6.dll"
$DllFullPath = (Resolve-Path $DllFullPath).Path

# Create registry entries for CLSID
$ClsidGuid = "{B6D63B6F-4584-4652-A02B-8D30AFAA61B1}"
$ClassesPath = "HKCU:\Software\Classes"

# Create CLSID registry entries
New-Item -Path "$ClassesPath\CLSID\$ClsidGuid" -Force | Out-Null
Set-ItemProperty -Path "$ClassesPath\CLSID\$ClsidGuid" -Name "(Default)" -Value "K6 Stroke"

New-Item -Path "$ClassesPath\CLSID\$ClsidGuid\InprocServer32" -Force | Out-Null
Set-ItemProperty -Path "$ClassesPath\CLSID\$ClsidGuid\InprocServer32" -Name "(Default)" -Value $DllFullPath
Set-ItemProperty -Path "$ClassesPath\CLSID\$ClsidGuid\InprocServer32" -Name "ThreadingModel" -Value "Apartment"

Write-Host "Installation complete!" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Reload the DLL in the current session:" -ForegroundColor Gray
Write-Host "   taskkill /F /IM ctfmon.exe" -ForegroundColor Gray
Write-Host "   Start-Process ctfmon.exe" -ForegroundColor Gray
Write-Host ""
Write-Host "2. Add the IME to Windows:" -ForegroundColor Gray
Write-Host "   - Go to Settings → Time & Language → Language & region" -ForegroundColor Gray
Write-Host "   - Select Chinese (Traditional, Hong Kong)" -ForegroundColor Gray
Write-Host "   - Click 'Add a keyboard'" -ForegroundColor Gray
Write-Host "   - Select 'K6' from the list" -ForegroundColor Gray
Write-Host ""
Write-Host "3. Switch input methods with Win+Space" -ForegroundColor Gray
Write-Host ""
Write-Host "Installation directory: $InstallDir" -ForegroundColor Gray
