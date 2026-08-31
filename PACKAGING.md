# 📦 GitMaster Packaging & Distribution Guide

Overview of packaging pipelines, formats, and distribution targets for GitMaster.

---

## 🚀 Cross-Platform Packaging & Distribution Matrix

| Packaging Target | Distribution / Platform | Status | Pipeline / Script Location | Details |
|---|---|---|---|---|
| **Arch Linux AUR** | Arch Linux / Manjaro | ✅ **Live** | [AUR `gitmaster`](https://aur.archlinux.org/packages/gitmaster) *(Separate git repo)* | `PKGBUILD` packaging for Arch users via `yay -S gitmaster` |
| **Flatpak** | Flathub / All Linux | ✅ **Ready** | [`org.gitmaster.GitMaster.yaml`](org.gitmaster.GitMaster.yaml), [`org.gitmaster.GitMaster.metainfo.xml`](org.gitmaster.GitMaster.metainfo.xml) | KDE/Qt6 6.8 SDK runtime, builds `libgit2`, AppStream validated |
| **AppImage** | Universal Linux | ✅ **Ready** | [`build_appimage.sh`](build_appimage.sh) | Bundles Qt6 & dependencies via `linuxdeploy` and `linuxdeploy-plugin-qt` |
| **Snap** | Canonical Snap Store | ✅ **Ready** | [`snap/snapcraft.yaml`](snap/snapcraft.yaml) | Uses `kde-neon-6` extension on Ubuntu `core24` |
| **RPM** | Fedora / RHEL / openSUSE | ✅ **Ready** | [`gitmaster.spec`](gitmaster.spec) | Standard RPM spec with AppStream metainfo & desktop integration |
| **macOS Bundle & DMG** | macOS (Apple Silicon & Intel) | ✅ **Ready** | [`macos/build.sh`](macos/build.sh), [`macos/build_icon.sh`](macos/build_icon.sh) | Creates `gitmaster.app`, bundles with `macdeployqt`, ad-hoc codesigns, outputs `.dmg` |
| **Windows Installer** | Windows 10 / 11 (x64) | ✅ **Ready** | [`windows/build_installer.bat`](windows/build_installer.bat), [`windows/gitmaster.iss`](windows/gitmaster.iss) | Inno Setup 6 installer packaging executable and Qt/OpenSSL DLLs |

---

## 🛠️ Build & Packaging Instructions

### 1. Arch Linux (AUR)
Maintained in the official AUR repository:
```bash
yay -S gitmaster
```

### 2. Flatpak (Flathub)
To build and install locally:
```bash
flatpak install -y flathub org.kde.Platform//6.8 org.kde.Sdk//6.8
flatpak-builder --user --install --force-clean build-flatpak org.gitmaster.GitMaster.yaml
flatpak run org.gitmaster.GitMaster
```

### 3. AppImage
Generates a standalone portable `.AppImage` in the `AppImage/` directory:
```bash
./build_appimage.sh
```

### 4. Snap
Builds a `.snap` package using Snapcraft:
```bash
snapcraft
```

### 5. Fedora / RPM
Build source RPM or binary package:
```bash
rpmbuild -ba gitmaster.spec
```

### 6. macOS DMG
Builds `gitmaster.app`, bundles dynamic frameworks via `macdeployqt`, and packages into a `.dmg`:
```bash
./macos/build.sh
```

### 7. Windows Installer (Inno Setup)
Build dependencies, compile the project, and create the setup executable:
```cmd
windows\build_deps.bat
windows\build.bat
windows\build_installer.bat
```
