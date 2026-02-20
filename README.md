# QFilmScanner
[![Platform](https://img.shields.io/badge/platform-Linux-blue)](https://kernel.org)
[![Qt](https://img.shields.io/badge/Qt-6.9+-41CD52?logo=qt)](https://www.qt.io/)
[![SANE](https://img.shields.io/badge/backend-SANE-orange)](http://www.sane-project.org/)
[![Build](https://img.shields.io/badge/build-CMake-064F8C?logo=cmake&logoColor=white)](https://cmake.org/)
[![License](https://img.shields.io/badge/license-GPLv3-blue)](https://www.gnu.org/licenses/gpl-3.0)
![GitHub release](https://img.shields.io/github/v/release/Spurdl/QFilmScanner)  

QFiSc - QT Film Scanner supporting **Plustek** and possibly other **SANE-supported flatbed devices**, on Linux!

## Screenshots

**Default view / Scan done:**  

<div style="display: flex; gap: 20px; align-items: flex-start;">
    <img src="images/Empty.png" alt="Empty" width="450">
    <img src="images/Scandone.png" alt="Scan Done" width="450">
</div>

---

## Installation via prebuilt binaries

This app does not bundle SANE inside the binary, and is required to be installed on the system itself.    
Include by adding in your package manager.  
 
**Arch**  
```sudo pacman -S sane```  

**Debian / Ubuntu**  
```sudo apt install sane```  

## Installation from package:

**Arch**
```sudo pacman -U qfilmscanner-*.pkg.tar.zst```  
Runtime dependencies are:  
- qt6-base 6.9+  
- qt6-imageformats  
- sane  

**Debian / Ubuntu**  
```sudo dpkg -i qfilmscanner_*.deb```
```sudo apt -f install```
Runtime dependencies are handled automatically:
- qt6-base
- qt6-image-formats-plugins
- libsane

## Usage 

Simply run as executable  

**Preview** does lowest found resolution scan, and does not save by default.  
Previewing does not require choosing a folder.

**Scan** does highest found resolution scan, and saves immediately after scan.

**Saving** overrides similarly named frame without prompt.

**Image edit** buttons do small convenience for later editing. Recommended is to use darktable for final colour correction.
- **R Reversal film**. Enabling disabled colour negation.
- **↻ Flip**. Enabling flips image to vertical.
- **⇄ Mirror** horizontally.
- **⇅ Mirror** vertically.
- **⎘ No depth scaling**. Removes auto-level scaling.

**Shortcuts** are used to quickly change frames. Arrow keys to right move frame counter up, and left move back.  
Space can be used to start scan immediately.

## Features

- Scan previews and final frames  
- Frame-by-frame scanning with automatic numbering  
- Support for **color / black & white**, **16-bit depth**, and basic image transforms (negate, rotate90, mirror horizontally, mirror vertically, auto-level 0.05/99.5%)  
- Save scans in multiple formats (`JPEG`, `PNG`, `TIFF`, `WEBP`, `BMP`) and save methods (`Raw`, `Edited`, `Raw + Edited`)  
- Embedded UI font for consistent look (Montserrat)  
- Status bar

## Tested devices

Tested devices list can be found from [here](docs/SUPPORTED.md).   
The build started for Plustek OpticFilm 7400, but supports any SANE-supported devices, found [here](http://www.sane-project.org/sane-mfgs.html#Z-PLUSTEK).  

## Building

This software requires some additional libraries  
- **QT 6.9+**  
- **CMake 3.16+**
- **C++17 compiler**

Additionally, it is required to install  
- **qt6-imageformats**
- **sane-backends**

Minimum dependencies are for arch-based distro   
```sudo pacman -S qt6-base qt6-imageformats sane cmake pkgconf base-devel```

For debian  
```sudo apt install build-essential cmake pkg-config qt6-base-dev qt6-image-formats-plugins libsane-dev```

### For deployment

Deployment requires additional software depending on build target.  
Packaging uses:
- linuxdeploy
- linuxdeploy-plugin-qt
- CPack (for .deb and .rpm)

## Coming features

 - Dark mode support dynamically from OS in development
 - 8-bit depth calculations for image transformations
 - *Your help* with testing and reporting other scanner times, resolutions and quirks. I will personally ever only own one anyway.

- - -
Made with QT6. Made with SANE-backend. Made with love, luck and freedom.