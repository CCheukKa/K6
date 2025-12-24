# K6
A [G6](http://www.miniapps.hk/g6code/) rewrite. You can check out the web version [here](https://cck.wtf/stroke).  

## Build

Requires Microsoft Visual Studio 2022 (MSVC) or MSVC Build Tools, and CMake 3.20+.

```powershell
# From repository root
cd K6

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

- Open Settings → Time & Language → Language & region → Chinese (Traditional, Hong Kong) → Preferred languages.
- Add a keyboard → look for "K6".
- Move it up or switch to it from the taskbar input switcher.

## Unregister (uninstall)

```powershell
cd scripts
./Unregister-IME.ps1 -BuildDir ..\build -Configuration Release
```

## Notes

- This is a simplified educational example. A production IME needs candidate UI, proper composition ranges, dictionaries, and settings.
- Built as a TSF TIP (Text Input Processor) COM in-proc server.
