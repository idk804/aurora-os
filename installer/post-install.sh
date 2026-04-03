#!/bin/bash
# Aurora OS Post-Installation Script
# Runs inside the chroot after base system installation

set -e

echo "🌌 Running Aurora OS post-install configuration..."

# ─── Set Aurora as default theme ───
mkdir -p /etc/gtk-4.0
cat > /etc/gtk-4.0/settings.ini << 'EOF'
[Settings]
gtk-theme-name=Aurora
gtk-icon-theme-name=Aurora
gtk-cursor-theme-name=Aurora
gtk-font-name=Inter 11
gtk-application-prefer-dark-theme=true
EOF

mkdir -p /etc/gtk-3.0
cat > /etc/gtk-3.0/settings.ini << 'EOF'
[Settings]
gtk-theme-name=Aurora
gtk-icon-theme-name=Aurora
gtk-cursor-theme-name=Aurora
gtk-font-name=Inter 11
gtk-application-prefer-dark-theme=true
EOF

# ─── Set Plymouth theme ───
if command -v plymouth-set-default-theme &>/dev/null; then
    plymouth-set-default-theme aurora
fi

# ─── Configure GRUB ───
if [ -f /etc/default/grub ]; then
    sed -i 's/GRUB_TIMEOUT=.*/GRUB_TIMEOUT=3/' /etc/default/grub
    sed -i 's/#\?GRUB_CMDLINE_LINUX_DEFAULT=.*/GRUB_CMDLINE_LINUX_DEFAULT="quiet splash loglevel=3"/' /etc/default/grub
    sed -i 's/#\?GRUB_GFXMODE=.*/GRUB_GFXMODE=auto/' /etc/default/grub
fi

# ─── Default wallpaper ───
mkdir -p /etc/skel/.config/aurora
cat > /etc/skel/.config/aurora/compositor.conf << 'EOF'
# Aurora Compositor Configuration
wallpaper=/usr/share/backgrounds/aurora/default.png
animations=true
animation_duration=250
border_width=2
corner_radius=10
inner_gap=8
outer_gap=12
natural_scroll=true
terminal=foot
launcher=aurora-launcher
EOF

# ─── Autostart desktop components ───
mkdir -p /etc/skel/.config/autostart

cat > /etc/skel/.config/autostart/aurora-panel.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=Aurora Panel
Exec=aurora-panel
NoDisplay=true
X-GNOME-Autostart-enabled=true
EOF

cat > /etc/skel/.config/autostart/aurora-widgets.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=Aurora Widgets
Exec=aurora-widgets
NoDisplay=true
X-GNOME-Autostart-enabled=true
EOF

# ─── Desktop entry for Aurora session ───
mkdir -p /usr/share/wayland-sessions
cat > /usr/share/wayland-sessions/aurora.desktop << 'EOF'
[Desktop Entry]
Name=Aurora
Comment=Aurora Desktop Environment
Exec=aurora-compositor -s "aurora-panel & aurora-widgets &"
Type=Application
DesktopNames=Aurora
EOF

# ─── System-wide environment ───
cat > /etc/environment.d/aurora.conf << 'EOF'
# Aurora OS environment
XDG_CURRENT_DESKTOP=Aurora
XDG_SESSION_TYPE=wayland
QT_QPA_PLATFORM=wayland
MOZ_ENABLE_WAYLAND=1
ELECTRON_OZONE_PLATFORM_HINT=wayland
EOF

# ─── Enable services ───
systemctl enable NetworkManager.service
systemctl enable bluetooth.service
systemctl enable power-profiles-daemon.service

# ─── SDDM or GDM as display manager (if installed) ───
if command -v sddm &>/dev/null; then
    systemctl enable sddm.service
elif command -v gdm &>/dev/null; then
    systemctl enable gdm.service
fi

echo "🌌 Aurora OS post-install complete!"
