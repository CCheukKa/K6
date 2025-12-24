@echo off
REM K6-IME-Setup.bat
REM Complete setup script for K6 IME - builds and installs without admin requirements
REM Usage: K6-IME-Setup.bat [admin|user]

setlocal enabledelayedexpansion

set PROJECT_ROOT=%~dp0..
if "%PROJECT_ROOT:~-1%"=="\" set PROJECT_ROOT=%PROJECT_ROOT:~0,-1%

set INSTALL_MODE=%1
if "%INSTALL_MODE%"=="" set INSTALL_MODE=user

echo.
echo ========================================
echo K6 Stroke IME Setup
echo ========================================
echo.

REM Detect if running with admin privileges
net session >nul 2>&1
set IS_ADMIN=0
if %errorlevel% equ 0 set IS_ADMIN=1

if "%INSTALL_MODE%"=="admin" (
    echo Installation Mode: System-wide (requires Admin)
    if %IS_ADMIN% equ 0 (
        echo.
        echo ERROR: This installation mode requires administrator privileges.
        echo Please run: "K6-IME-Setup.bat admin"
        echo And select "Yes" when prompted for admin rights.
        echo.
        pause
        exit /b 1
    )
) else (
    echo Installation Mode: User-only (no admin required)
)

echo.
echo Step 1: Checking for CMake...
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found. Please install CMake 3.20 or later.
    echo Visit: https://cmake.org/download/
    pause
    exit /b 1
)
echo ✓ CMake found

echo.
echo Step 2: Checking for Visual Studio Build Tools...
if not exist "C:\Program Files (x86)\Microsoft Visual Studio\2022" (
    if not exist "C:\Program Files\Microsoft Visual Studio\2022" (
        echo WARNING: Visual Studio 2022 not found in standard locations
        echo Build may fail. Please install Visual Studio 2022 or MSVC Build Tools.
    ) else (
        echo ✓ Visual Studio 2022 found
    )
) else (
    echo ✓ Visual Studio 2022 found
)

echo.
echo Step 3: Building K6 IME (Release configuration)...
pushd "%PROJECT_ROOT%"

if exist "build" (
    echo Cleaning previous build...
    rmdir /s /q build >nul 2>&1
)

echo Configuring CMake...
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)
echo ✓ CMake configured

echo Building...
cmake --build build --config Release --parallel
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)
echo ✓ Build successful

popd

REM Check if DLL was created
set DLL_PATH=%PROJECT_ROOT%\build\Release\K6.dll
if not exist "%DLL_PATH%" (
    echo ERROR: K6.dll not found at %DLL_PATH%
    pause
    exit /b 1
)

echo.
echo Step 4: Installing K6 IME...
echo.

if "%INSTALL_MODE%"=="admin" (
    REM System-wide installation using regsvr32
    echo Installing for all users...
    if not exist "%PROGRAMFILES%\K6IME" mkdir "%PROGRAMFILES%\K6IME"
    copy /Y "%DLL_PATH%" "%PROGRAMFILES%\K6IME\K6.dll" >nul
    copy /Y "%PROJECT_ROOT%\data\*" "%PROGRAMFILES%\K6IME\" >nul
    
    echo Registering COM server...
    regsvr32 /s "%PROGRAMFILES%\K6IME\K6.dll"
    if %errorlevel% equ 0 (
        echo ✓ Installation successful
        echo IME installed to: %PROGRAMFILES%\K6IME
    ) else (
        echo ERROR: COM registration failed
        pause
        exit /b 1
    )
) else (
    REM User-only installation using PowerShell
    echo Installing for current user...
    powershell -NoProfile -ExecutionPolicy Bypass -File "%PROJECT_ROOT%\scripts\Install-IME-NoAdmin.ps1" -DllPath "%DLL_PATH%"
    if %errorlevel% neq 0 (
        echo ERROR: Installation failed
        pause
        exit /b 1
    )
)

echo.
echo Step 5: Configuring Windows Input Method...
echo.
echo Please follow these steps to enable K6 in Windows:
echo.
echo 1. Press Windows key + I to open Settings
echo 2. Go to: Time ^& Language ^> Language ^& region
echo 3. Under "Languages" find and select: Chinese (Traditional, Hong Kong)
echo 4. Click the three dots (...) next to it and select "Language options"
echo 5. Under "Keyboards" click "Add a keyboard"
echo 6. Find and select "K6" from the list
echo 7. You can now use Windows key + Space to switch between input methods
echo.

echo.
echo ========================================
echo Setup Complete!
echo ========================================
echo.
echo To use K6 IME:
echo - Press Windows key + Space to switch input methods
echo - Use Win + Shift + Space to cycle through your input methods
echo.
echo To uninstall:
if "%INSTALL_MODE%"=="admin" (
    echo - Run: "%PROJECT_ROOT%\scripts\Unregister.ps1"
) else (
    echo - Run: "%PROJECT_ROOT%\scripts\Uninstall-IME-NoAdmin.ps1"
)
echo.

pause
