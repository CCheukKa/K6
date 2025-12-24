; K6IME-Installer.nsi
; NSIS Installer for K6 IME with user-mode installation support

!include "MUI2.nsh"
!include "x64.nsh"

; Configuration
!define PRODUCT_NAME "K6 Stroke IME"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "CCheukKa"
!define PRODUCT_WEB_SITE "https://github.com/CCheukKa/K6"

; Installer attributes
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "K6IME-${PRODUCT_VERSION}-installer.exe"
InstallDir "$PROGRAMFILES\K6IME"

; MUI Settings
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; Installer sections
Section "K6 IME"
    SetOutPath "$INSTDIR"
    
    ; Copy DLL and data files
    File "build\Release\K6.dll"
    File "data\punctuationData.txt"
    File "data\strokeData.txt"
    File "data\suggestionsData.txt"
    
    ; Register COM server (requires admin)
    Exec 'regsvr32 /s "$INSTDIR\K6.dll"'
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; Create shortcuts
    CreateDirectory "$SMPROGRAMS\K6 IME"
    CreateShortcut "$SMPROGRAMS\K6 IME\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    
SectionEnd

; Uninstaller section
Section "Uninstall"
    ; Unregister COM server
    ExecWait 'regsvr32 /u /s "$INSTDIR\K6.dll"'
    
    ; Remove files
    Delete "$INSTDIR\K6.dll"
    Delete "$INSTDIR\punctuationData.txt"
    Delete "$INSTDIR\strokeData.txt"
    Delete "$INSTDIR\suggestionsData.txt"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"
    
    ; Remove shortcuts
    RMDir /r "$SMPROGRAMS\K6 IME"
SectionEnd
