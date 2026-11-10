# KhinsiderDownloader 🎵 
### A simple Bulk Khinsider downloader with GUI

> **This is FrankenSama's maintained version** of KhinsiderDownloader, developed independently with its own changes and release schedule. It is based on the original [KhinsiderDownloader](https://github.com/weespin/KhinsiderDownloader) by [Weespin](https://github.com/weespin) — many thanks for the original work.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Github Downloads (total)](https://img.shields.io/github/downloads/FrankenSama/KhinsiderDownloader/total.svg)](https://tooomm.github.io/github-release-stats/?username=FrankenSama&repository=KhinsiderDownloader)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/FrankenSama/KhinsiderDownloader/build.yml)
# Screenshots
![Screenshot](https://weesp.in/i/309ce634ce4115ee.png)

## ✨ Features

 - 🎯 Windows and Linux (AppImage) Builds.
 - 🔎 Search albums easily
 - ⚡ Async downloading with worker pool for faster performance
 - 🎨 Simple and clean GUI
 - 🌙 Nightly builds available

## 📥 Download

-   [Latest Stable Release (Windows & Linux)](https://github.com/FrankenSama/KhinsiderDownloader/releases) 
    
-   [Nightly Development Builds (Artifact)](https://nightly.link/FrankenSama/KhinsiderDownloader/workflows/build/dev)

### 🔀 Release Channels

| Channel | Branch | Description |
|---|---|---|
| **Stable** | `master` | Tested releases, cut from `master`. Recommended for normal use. |
| **Nightly** | `dev` | Latest development builds from `dev`. May be unstable or broken. |

Development happens on `dev`; changes are merged into `master` only when they are ready for a stable release.

## 🌟 Star the Project
If you like KhinsiderDownloader, feel free to leave a star on GitHub to show your support!

## 🛠️ Building From Source

### Windows
#### Requirements

-  🛠️ CMake 3.16+
-  🖼️ Qt >=6.12.0  
-  🧰 Visual Studio 2022
-  📦 VCPKG (clone manually, preferred at `C:\vcpkg` for automatic builds. Don’t forget to run `bootstrap-vcpkg.bat`)

Manual Building

```bash
git clone https://github.com/FrankenSama/KhinsiderDownloader.git --branch master
cd KhinsiderDownloader
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="path\to\Qt" -DCMAKE_TOOLCHAIN_FILE="path\to\vcpkg"
cmake --build . --config Release
"path\to\qt\bin\windeployqt.exe" --qmldir "\path\to\project\src\ui" --no-translations --release --force-openssl "\path\to\project\build\Release\appKhinsiderQT.exe"
```

Automatic Building

```bash
git clone https://github.com/FrankenSama/KhinsiderDownloader.git --branch master
cd KhinsiderDownloader/scripts/windows
fullbuild.bat
```

### Linux
#### Requirements

-  🛠️ CMake 3.16+
-  🖼️ Qt >=6.12.0
  
<details>
 <summary>Prerequisites for debian</summary>
 
```text
 sudo apt install qt6-base-dev cmake build-essential qtdeclarative5-dev qt6-declarative-dev libqt6xml6 libcurl4-openssl-dev qml6-module-qt5compat-graphicaleffects
```
</details>

```bash
git clone -b master https://github.com/FrankenSama/KhinsiderDownloader.git
cd KhinsiderDownloader
mkdir build && cd build
cmake ..
make -j$(nproc)
cd ..
chmod +x scripts/linux/deploy.sh
./scripts/linux/deploy.sh
```

## 🤝 Contributing

Contributions are very welcome!  
You can help by:

-   🐛 Reporting bugs
    
-   💡 Suggesting enhancements
    
-   🎨 Noticing UI/UX issues
    
-   🛠️ Opening pull requests
    

If you want to contribute code:

1.  Fork the repository
    
2.  Create a feature branch (`git checkout -b your-feature`)
    
3.  Commit your changes
    
4.  Push to your branch
    
5.  Create a Pull Request

Found a bug?
 👉 [Open an Issue here](https://github.com/FrankenSama/KhinsiderDownloader/issues)

## ⚖️ Legal Disclaimer and Copyright Notice

This application downloads music directly from [https://downloads.khinsider.com/](https://downloads.khinsider.com/). All music and related content are hosted and provided by the external website.

Similar to torrent apps, this application does not host or store any music files on the server. It merely facilitates the download process from the external source. All music content is subject to the terms and conditions of the copyright holders and the external website.

If you have any copyright concerns or wish to file a DMCA notice, please contact [Khinsider](https://downloads.khinsider.com/) directly. Any copyright issues should be reported to them, not to the developer of this application.

Please respect the intellectual property rights of artists and content creators.

## 📜 License

**KhinsiderDownloader**  
Original work Copyright (C) 2025 Weespin  
Modifications Copyright (C) 2026 FrankenSama

This program is free software: you can redistribute it and/or modify it under the terms of the  
GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,  
but **WITHOUT ANY WARRANTY**; without even the implied warranty of **MERCHANTABILITY** or **FITNESS FOR A PARTICULAR PURPOSE**.  
See the GNU General Public License for more details.


You should have received a copy of the GNU General Public License along with this program.  
If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).
