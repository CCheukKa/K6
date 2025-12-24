# K6 Stroke IME

A [G6](http://www.miniapps.hk/g6code/) rewrite for Windows. Input Method Editor (IME) for stroke-based Chinese input.  
Web version: [https://cck.wtf/stroke](https://cck.wtf/stroke)

---

## 🚀 Quick Start

**Non-admin installation (recommended):**
1. Run the setup script from the project root:
   ```batch
   K6-IME-Setup.bat
   ```
   This automatically builds and installs K6 IME for the current user.

2. Add K6 to Windows Settings:
   - Press `Win + I` to open Settings.
   - Go to **Time & Language** → **Language & region**.
   - Select **Chinese (Traditional, Hong Kong)**.
   - Click **Add a keyboard** and select **K6**.
   - Use `Win + Space` to switch input methods.

---

## 🛠️ Installation Methods

### 1. One-Click Setup (Easiest)
Run `K6-IME-Setup.bat` from the root directory. It detects admin privileges and performs a complete setup (build + install).

### 2. Manual Non-Admin Installation (User-mode)
Perfect for users without administrator rights.
```powershell
cd scripts
.\Build.ps1
.\Install-IME-NoAdmin.ps1
```
- **Location:** `%LOCALAPPDATA%\K6IME\`
- **Registry:** `HKCU:\Software\Classes\CLSID\`

### 3. System-Wide Installation (Admin Required)
Installs K6 for all users on the system.
```powershell
# Run PowerShell as Administrator
cd scripts
.\Build.ps1
.\Register.ps1
```
- **Location:** `%PROGRAMFILES%\K6IME\`
- **Registry:** `HKCR:\CLSID\`

### 4. NSIS Installer (Windows Installer)
For distribution, you can create a standalone `.exe` installer.
```cmd
cd scripts
.\Build.ps1 -Configuration Release
makensis.exe K6IME-Installer.nsi
```

---

## 📜 Script Reference

| Script                        | Purpose                   | Admin? | Usage                           |
| :---------------------------- | :------------------------ | :----- | :------------------------------ |
| `K6-IME-Setup.bat`            | One-click build + install | ❌/✅    | `K6-IME-Setup.bat`              |
| `Build.ps1`                   | Build the DLL             | ❌      | `.\Build.ps1`                   |
| `Register.ps1`                | System-wide registration  | ✅      | `.\Register.ps1`                |
| `Install-IME-NoAdmin.ps1`     | User-only installation    | ❌      | `.\Install-IME-NoAdmin.ps1`     |
| `Uninstall-IME-NoAdmin.ps1`   | User-only removal         | ❌      | `.\Uninstall-IME-NoAdmin.ps1`   |
| `Unregister.ps1`              | System-wide removal       | ✅      | `.\Unregister.ps1`              |
| `Verify-IME-Installation.ps1` | Check installation status | ❌      | `.\Verify-IME-Installation.ps1` |

---

## 🔍 Troubleshooting

### IME Not Appearing
1. **Restart Input Method Manager:**
   ```powershell
   taskkill /F /IM ctfmon.exe
   Start-Process ctfmon.exe
   ```
2. **Restart Explorer:**
   ```powershell
   taskkill /F /IM explorer.exe
   Start-Process explorer.exe
   ```

### DLL Not Found
- Ensure you ran `Build.ps1` first.
- Check `build\Release\K6.dll` exists.

### Registration Failed
- Run PowerShell as Administrator for system-wide registration.
- Ensure no other processes are locking the DLL.

---

## ⚙️ Technical Details

- **Architecture:** TSF TIP (Text Input Processor) COM in-proc server.
- **Language:** Traditional Chinese (Hong Kong variant).
- **Input System:** Stroke-based.
- **Features:**
  - ✅ No admin required for user-mode install.
  - ✅ Easy to uninstall.
  - ✅ Diagnostic tools included.

---

## 📋 Requirements

### For Users
- Windows 10 or later (x64).
- PowerShell 5.0 or later.

### For Developers
- Visual Studio 2022 or MSVC Build Tools.
- CMake 3.20 or later.

---

## 🤝 Support & Contribution

- **Issues:** [GitHub Issues](https://github.com/CCheukKa/K6/issues)
- **Original G6:** [http://www.miniapps.hk/g6code/](http://www.miniapps.hk/g6code/)

