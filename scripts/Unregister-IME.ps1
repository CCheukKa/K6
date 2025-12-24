Param(
    [string]$BuildDir = "..\build",
    [string]$Configuration = "Release"
)

$ErrorActionPreference = 'Stop'

$modulePath = Join-Path $BuildDir "$Configuration\ChineseIMEHKExample.dll"
if (-not (Test-Path $modulePath)) {
    Write-Warning "DLL not found at $modulePath. Proceeding to unregister via regsvr32 /u anyway if path is correct."
}

Write-Host "Unregistering IME: $modulePath" -ForegroundColor Cyan

Start-Process -FilePath "regsvr32.exe" -ArgumentList "/s", "/u", "\"$modulePath\"" -Wait -NoNewWindow

Write-Host "Unregistered. You may need to restart ctfmon.exe or sign out/in." -ForegroundColor Green
