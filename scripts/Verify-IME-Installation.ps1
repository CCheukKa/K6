# Verify-IME-Installation.ps1
# Validate K6 IME installation status and troubleshoot common issues

$ErrorActionPreference = "Continue"

Write-Host "=== K6 IME Installation Verification ===" -ForegroundColor Cyan
Write-Host ""

# Detect installation type
$UserInstallDir = Join-Path $env:LOCALAPPDATA "K6IME"
$AdminInstallDir = Join-Path $env:PROGRAMFILES "K6IME"
$UserRegistryPath = "HKCU:\Software\Classes\CLSID\{B6D63B6F-4584-4652-A02B-8D30AFAA61B1}"
$AdminRegistryPath = "HKCR:\CLSID\{B6D63B6F-4584-4652-A02B-8D30AFAA61B1}"

$UserDllPath = Join-Path $UserInstallDir "K6.dll"
$AdminDllPath = Join-Path $AdminInstallDir "K6.dll"

$InstallationType = "Not Installed"
$DllLocation = $null
$IsWorking = $false

Write-Host "1. Checking Installation..." -ForegroundColor Yellow
Write-Host ""

# Check user installation
if (Test-Path $UserDllPath) {
    Write-Host "✓ User-mode installation found" -ForegroundColor Green
    Write-Host "  Location: $UserInstallDir" -ForegroundColor Gray
    $InstallationType = "User"
    $DllLocation = $UserDllPath
    
    if (Test-Path $UserRegistryPath) {
        Write-Host "✓ User registry entries found" -ForegroundColor Green
        $IsWorking = $true
    } else {
        Write-Host "✗ User registry entries missing" -ForegroundColor Red
    }
}

# Check admin installation
if (Test-Path $AdminDllPath) {
    Write-Host "✓ System-wide installation found" -ForegroundColor Green
    Write-Host "  Location: $AdminInstallDir" -ForegroundColor Gray
    $InstallationType = "Admin"
    $DllLocation = $AdminDllPath
    
    if (Test-Path $AdminRegistryPath) {
        Write-Host "✓ System registry entries found" -ForegroundColor Green
        $IsWorking = $true
    } else {
        Write-Host "✗ System registry entries missing" -ForegroundColor Red
    }
}

if ($InstallationType -eq "Not Installed") {
    Write-Host "✗ K6 IME not found on this system" -ForegroundColor Red
    Write-Host ""
    Write-Host "To install K6, run:" -ForegroundColor Yellow
    Write-Host "  .\K6-IME-Setup.bat" -ForegroundColor Cyan
    exit 1
}

Write-Host ""
Write-Host "2. Checking DLL Status..." -ForegroundColor Yellow
Write-Host ""

if (Test-Path $DllLocation) {
    $FileInfo = Get-Item $DllLocation
    $Version = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($DllLocation)
    
    Write-Host "✓ DLL file exists" -ForegroundColor Green
    Write-Host "  File: $($FileInfo.Name)" -ForegroundColor Gray
    Write-Host "  Size: $([math]::Round($FileInfo.Length / 1KB, 2)) KB" -ForegroundColor Gray
    Write-Host "  Version: $($Version.FileVersion)" -ForegroundColor Gray
    Write-Host "  Modified: $($FileInfo.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "✗ DLL file not found" -ForegroundColor Red
}

# Check data files
Write-Host ""
Write-Host "3. Checking Data Files..." -ForegroundColor Yellow
Write-Host ""

$DataDir = if ($InstallationType -eq "User") { $UserInstallDir } else { $AdminInstallDir }
$RequiredFiles = @("punctuationData.txt", "strokeData.txt", "suggestionsData.txt")
$AllDataFilesPresent = $true

foreach ($file in $RequiredFiles) {
    $filePath = Join-Path $DataDir $file
    if (Test-Path $filePath) {
        $fileSize = (Get-Item $filePath).Length / 1KB
        Write-Host "✓ $file ($(([math]::Round($fileSize, 2))) KB)" -ForegroundColor Green
    } else {
        Write-Host "✗ $file missing" -ForegroundColor Red
        $AllDataFilesPresent = $false
    }
}

# Check Windows input methods registration
Write-Host ""
Write-Host "4. Checking Windows Input Method Registration..." -ForegroundColor Yellow
Write-Host ""

# Get Windows language registry path
$langKey = "HKCU:\Keyboard Layout\Substitutes"
$tsfKey = "HKCU:\Software\Microsoft\CTF"

if (Test-Path $langKey) {
    $substitutes = Get-ItemProperty $langKey -ErrorAction SilentlyContinue
    if ($substitutes) {
        Write-Host "✓ Keyboard substitutes found" -ForegroundColor Green
    }
}

if (Test-Path $tsfKey) {
    Write-Host "✓ Text Services Framework registered" -ForegroundColor Green
} else {
    Write-Host "⚠ Text Services Framework entries not found" -ForegroundColor Yellow
}

# Check TSF service
Write-Host ""
Write-Host "5. Checking System Services..." -ForegroundColor Yellow
Write-Host ""

$ctfmonProcess = Get-Process "ctfmon" -ErrorAction SilentlyContinue
if ($ctfmonProcess) {
    Write-Host "✓ ctfmon.exe (Input Method Manager) is running" -ForegroundColor Green
    Write-Host "  PID: $($ctfmonProcess.Id)" -ForegroundColor Gray
} else {
    Write-Host "✗ ctfmon.exe is not running" -ForegroundColor Red
    Write-Host "  This may cause input method to not work properly" -ForegroundColor Yellow
}

# Summary
Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host ""

if ($IsWorking) {
    Write-Host "✓ K6 IME appears to be installed correctly" -ForegroundColor Green
    Write-Host ""
    Write-Host "To use K6:" -ForegroundColor Cyan
    Write-Host "1. Go to Settings → Time & Language → Language & region" -ForegroundColor Gray
    Write-Host "2. Select Chinese (Traditional, Hong Kong)" -ForegroundColor Gray
    Write-Host "3. Add 'K6' from the keyboard list" -ForegroundColor Gray
    Write-Host "4. Press Win+Space to switch input methods" -ForegroundColor Gray
} else {
    Write-Host "✗ K6 IME installation may have issues" -ForegroundColor Red
    Write-Host ""
    Write-Host "Recommended fixes:" -ForegroundColor Yellow
    
    if (-not $ctfmonProcess) {
        Write-Host "• Restart Input Method Manager:" -ForegroundColor Gray
        Write-Host "  taskkill /F /IM ctfmon.exe && Start-Process ctfmon.exe" -ForegroundColor Cyan
    }
    
    if (-not $AllDataFilesPresent) {
        Write-Host "• Reinstall K6:" -ForegroundColor Gray
        Write-Host "  .\K6-IME-Setup.bat" -ForegroundColor Cyan
    }
}

Write-Host ""
Write-Host "Installation Type: $InstallationType" -ForegroundColor Gray
Write-Host "Installation Path: $(if ($DllLocation) { Split-Path $DllLocation } else { 'N/A' })" -ForegroundColor Gray
Write-Host ""
