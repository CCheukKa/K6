# 中文輸入法範例 (繁體)

A minimal Text Services Framework (TSF) keyboard TIP (IME) for Windows 11 (Traditional Chinese). It captures alphabetic keys, shows inline preedit, and commits a few demo mappings like `ni` → 你, `hao` → 好 when you press space.

## Build

Requires Microsoft Visual Studio 2022 (MSVC) or MSVC Build Tools, and CMake 3.20+.

```powershell
# From repository root
cd ChineseIME-HK-Example

# Configure for x64 MSVC
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# Build Release
cmake --build build --config Release
```

## Register (install)

```powershell
cd scripts
./Register-IME.ps1 -BuildDir ..\build -Configuration Release
```

This calls `regsvr32` which triggers `DllRegisterServer` to register the TSF TIP profile.

You may need to restart `ctfmon.exe` or sign out/in for changes to appear.

## Enable in Windows

- Open Settings → Time & Language → Language & region → Chinese (Traditional, Taiwan) → Preferred languages.
- Add a keyboard → look for "繁中示例 IME".
- Move it up or switch to it from the taskbar input switcher.

## Use

- Type alphabetic characters (e.g., `ni` then space) → commits 你.
- Demo mappings include: `ni`→你, `hao`→好, `zhong`→中, `guo`→國, `tai`→台, `wan`→灣, `shi`→是, `de`→的, `ren`→人.
- Backspace edits the inline preedit.

## Unregister (uninstall)

```powershell
cd scripts
./Unregister-IME.ps1 -BuildDir ..\build -Configuration Release
```

## Notes

- This is a simplified educational example. A production IME needs candidate UI, proper composition ranges, dictionaries, and settings.
- Built as a TSF TIP (Text Input Processor) COM in-proc server.
