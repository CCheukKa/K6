@echo off
REM K6-IME-Setup.bat
REM Launcher script - delegates to scripts\K6-IME-Setup.bat
REM This is a convenience wrapper to run setup from the project root

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0scripts
set SETUP_SCRIPT=%SCRIPT_DIR%\K6-IME-Setup.bat

if not exist "%SETUP_SCRIPT%" (
    echo ERROR: Setup script not found at %SETUP_SCRIPT%
    echo Please ensure the scripts folder exists
    pause
    exit /b 1
)

REM Pass all arguments to the actual setup script
call "%SETUP_SCRIPT%" %*
