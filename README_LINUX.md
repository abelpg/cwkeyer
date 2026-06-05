# CwKeyer — Linux Build Guide

## 1. Install System Dependencies

**Debian / Ubuntu / Mint**
```bash
sudo apt install -y \
    build-essential cmake ninja-build \
    libusb-1.0-0-dev \
    libgl1-mesa-dev \
    pkg-config
```

**Fedora / RHEL**
```bash
sudo dnf install -y \
    gcc-c++ cmake ninja-build \
    libusb1-devel \
    mesa-libGL-devel \
    pkg-config
```

---

## 2. Install Qt 6.11+

**Option A — Qt Online Installer (recommended)**

Download from https://www.qt.io/download-qt-installer, then:
```bash
chmod +x qt-online-installer-linux-*.run
./qt-online-installer-linux-*.run
```
Select: **Qt 6.11 → Desktop gcc 64-bit** and add modules **Qt Multimedia** and **Qt Quick**.

**Option B — aqtinstall (no Qt account required)**
```bash
pip install aqtinstall
aqt install-qt linux desktop 6.11.1 gcc_64 \
    -m qtmultimedia qtquick3d
```

---

## 3. Configure and Build

```bash
cd ~/CwKeyer

# Configure — adjust the Qt path to match your installation
cmake -B build-linux \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.11.1/gcc_64 \
      -G Ninja

# Build
cmake --build build-linux --parallel
```

---

## 4. Set Required Linux Permissions (once)

These permissions are needed for the keyboard hook (evdev), key injection (uinput),
and serial port access. Run once, then **log out and back in** (or reboot).

```bash
# Keyboard listener — read /dev/input/event* devices
sudo usermod -aG input $USER

# Keyboard injection — write to /dev/uinput
sudo usermod -aG uinput $USER
echo 'KERNEL=="uinput", GROUP="uinput", MODE="0660"' \
    | sudo tee /etc/udev/rules.d/99-uinput.rules
sudo udevadm control --reload-rules

# Serial ports — access /dev/ttyUSB*, /dev/ttyACM*, /dev/ttyS*
sudo usermod -aG dialout $USER

# Apply groups without rebooting (current shell only)
newgrp input
```

---

## 5. Run

```bash
./build-linux/CwKeyerApp
```

---

## 6. Optional package dependencies
```bash
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy*.AppImage

cmake -B build-linux \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.11.1/gcc_64 \
      -G Ninja
cmake --build build-linux --parallel

mkdir -p AppDir/usr/bin
cp build-linux/CwKeyerApp AppDir/usr/bin/

# Eliminar plugins SQL no necesarios
#rm -f AppDir/usr/plugins/sqldrivers/libqsqlmimer.so
#rm -rf AppDir/usr/plugins/sqldrivers/

export QMAKE=/opt/Qt/6.11.1/gcc_64/bin/qmake
export QML_SOURCES_PATHS=$PWD  # para que incluya los QML
export EXCLUDE_LIBS="libqsqlmimer.so"
./linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --executable AppDir/usr/bin/CwKeyerApp \
    --desktop-file CwKeyer.desktop \
    --icon-file CwKeyer.png \
    --plugin qt \
    --output appimage \
    --exclude-library "libqsqlmimer*" \
    --exclude-library "libqsqlodbc*" \
    --exclude-library "libqsqlpsql*" \
    --exclude-library "libqsqlite*"

```

## 6. Run package
```bash

apt install \
    libfontconfig1-dev \
    libfreetype-dev \
    libgtk-3-dev \
    libx11-dev \
    libx11-xcb-dev \
    libxcb-cursor-dev \
    libxcb-glx0-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-shape0-dev \
    libxcb-shm0-dev \
    libxcb-sync-dev \
    libxcb-util-dev \
    libxcb-xfixes0-dev \
    libxcb-xkb-dev \
    libxcb1-dev \
    libxext-dev \
    libxfixes-dev \
    libxi-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libxrender-dev \
    ffmpeg \
    libopengl0 \
    libxcb-cursor0 \
    libxcb-cursor-dev \
    qml6-module-qtquick-controls
```

## Dependency Summary

| Component | Requirement |
|-----------|-------------|
| `KeyboardListener` (global key capture) | `/dev/input/event*` — `input` group |
| `Keyboard` (key injection) | `/dev/uinput` — `uinput` group |
| `SerialComm` / `N1MMProxy` | `/dev/ttyUSB*`, `/dev/ttyACM*` — `dialout` group |
| USB devices | `libusb-1.0-0-dev` (system package) |
| Qt framework | Qt 6.11+ with Multimedia and Quick modules |

