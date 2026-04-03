#!/bin/bash
# Aurora OS ISO Build Script
# Requires: archiso package installed on an Arch Linux system

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="/tmp/aurora-build"
OUT_DIR="$PROJECT_ROOT/out"
PROFILE_DIR="$BUILD_DIR/profile"

echo "╔══════════════════════════════════════╗"
echo "║       Aurora OS Build System         ║"
echo "║            🌌 v0.1.0                 ║"
echo "╚══════════════════════════════════════╝"

# Clean previous builds
echo "[*] Cleaning previous builds..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR" "$OUT_DIR"

# Copy archiso releng profile as base
echo "[*] Setting up archiso profile..."
cp -r /usr/share/archiso/configs/releng "$PROFILE_DIR"

# Override package list
echo "[*] Applying Aurora package list..."
cp "$SCRIPT_DIR/packages.x86_64" "$PROFILE_DIR/packages.x86_64"

# Copy Aurora customizations into airootfs
echo "[*] Copying Aurora customizations..."
AIROOTFS="$PROFILE_DIR/airootfs"

# Plymouth theme
if [ -d "$PROJECT_ROOT/themes/plymouth" ]; then
    mkdir -p "$AIROOTFS/usr/share/plymouth/themes/aurora"
    cp -r "$PROJECT_ROOT/themes/plymouth/"* "$AIROOTFS/usr/share/plymouth/themes/aurora/"
fi

# GTK theme
if [ -d "$PROJECT_ROOT/themes/gtk" ]; then
    mkdir -p "$AIROOTFS/usr/share/themes/Aurora"
    cp -r "$PROJECT_ROOT/themes/gtk/"* "$AIROOTFS/usr/share/themes/Aurora/"
fi

# Icons
if [ -d "$PROJECT_ROOT/themes/icons" ]; then
    mkdir -p "$AIROOTFS/usr/share/icons/Aurora"
    cp -r "$PROJECT_ROOT/themes/icons/"* "$AIROOTFS/usr/share/icons/Aurora/"
fi

# Wallpapers
if [ -d "$PROJECT_ROOT/themes/wallpapers" ]; then
    mkdir -p "$AIROOTFS/usr/share/backgrounds/aurora"
    cp -r "$PROJECT_ROOT/themes/wallpapers/"* "$AIROOTFS/usr/share/backgrounds/aurora/"
fi

# Branding
if [ -d "$PROJECT_ROOT/branding" ]; then
    mkdir -p "$AIROOTFS/usr/share/aurora/branding"
    cp -r "$PROJECT_ROOT/branding/"* "$AIROOTFS/usr/share/aurora/branding/"
fi

# Desktop environment
if [ -d "$PROJECT_ROOT/desktop-environment" ]; then
    mkdir -p "$AIROOTFS/usr/share/aurora/desktop"
    cp -r "$PROJECT_ROOT/desktop-environment/"* "$AIROOTFS/usr/share/aurora/desktop/"
fi

# Set Aurora branding in profile
sed -i "s/iso_name=.*/iso_name='aurora-os'/" "$PROFILE_DIR/profiledef.sh"
sed -i "s/iso_label=.*/iso_label='AURORA_OS'/" "$PROFILE_DIR/profiledef.sh"
sed -i "s/iso_publisher=.*/iso_publisher='Aurora OS Project'/" "$PROFILE_DIR/profiledef.sh"

# Enable services
mkdir -p "$AIROOTFS/etc/systemd/system/multi-user.target.wants"
ln -sf /usr/lib/systemd/system/NetworkManager.service \
    "$AIROOTFS/etc/systemd/system/multi-user.target.wants/NetworkManager.service"
ln -sf /usr/lib/systemd/system/bluetooth.service \
    "$AIROOTFS/etc/systemd/system/multi-user.target.wants/bluetooth.service"

# Build ISO
echo "[*] Building ISO (this will take a while)..."
sudo mkarchiso -v -w "$BUILD_DIR/work" -o "$OUT_DIR" "$PROFILE_DIR"

echo ""
echo "╔══════════════════════════════════════╗"
echo "║     Build complete! 🌌              ║"
echo "║     ISO: $OUT_DIR/               ║"
echo "╚══════════════════════════════════════╝"
