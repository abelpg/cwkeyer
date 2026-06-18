#!/bin/bash
set -e
rm -rf build-linux
cmake -B build-linux -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/Qt/6.11.1/gcc_64 -G Ninja
cmake --build build-linux --parallel

rm -rf AppDir

export QMAKE=/opt/Qt/6.11.1/gcc_64/bin/qmake
export QML_SOURCES_PATHS=$PWD
export EXCLUDE_LIBS="libqsqlmimer.so"

# Preparar directorios
mkdir -p AppDir/usr/bin

# Copiar ejecutable
cp build-linux/CwKeyerApp AppDir/usr/bin/

# Ejecutar linuxdeploy
./linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --icon-file CwKeyerApp.png \
    --desktop-file CwKeyerApp.desktop \
    --executable AppDir/usr/bin/CwKeyerApp \
    --plugin qt \
    --output appimage \
    --exclude-library "libqsqlmimer*" \
    --exclude-library "libqsqlodbc*" \
    --exclude-library "libqsqlpsql*" \
    --exclude-library "libqsqlite*"

# Empaquetar (opcional)
tar -czf CwKeyer-linux.tar.gz AppDir