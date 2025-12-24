Param(
    [string]$BuildDir = "..\build",
    [string]$Configuration = "Release"
)

$ErrorActionPreference = 'Stop'

$modulePath = Join-Path $BuildDir "$Configuration\ChineseIMEHKExample.dll"
if (-not (Test-Path $modulePath)) {
    Write-Error "DLL not found at $modulePath. Build first."
}

Write-Host "Registering IME: $modulePath" -ForegroundColor Cyan

# Register COM server which triggers DllRegisterServer to register the TIP
Start-Process -FilePath "regsvr32.exe" -ArgumentList "/s", "\"$modulePath\"" -Wait -NoNewWindow

Write-Host "Registered. You may need to restart ctfmon.exe or sign out/in." -ForegroundColor Green
