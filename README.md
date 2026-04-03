# Aurora OS 🌌

An Arch Linux-based operating system featuring a fully custom desktop environment with aurora borealis-inspired theming.

## Vision

Aurora OS is a beautiful, modern operating system built on top of Arch Linux, featuring:

- **Aurora Desktop Environment (ADE)** — a custom Wayland compositor and desktop shell
- **Aurora Theme System** — GTK theme, icon pack, cursors, and Plymouth boot splash
- **Aurora Widgets** — desktop widgets (clock, weather, system monitor, media player)
- **Aurora App Launcher** — a sleek application launcher with search
- **Aurora Settings** — unified system settings application

## Architecture

- **Base:** Arch Linux (rolling release)
- **Display Protocol:** Wayland (via wlroots)
- **Compositor:** Custom wlroots-based compositor
- **Toolkit:** GTK4 + libadwaita (apps), custom rendering for shell components
- **Language:** C (compositor), Python/Vala (desktop shell & apps)
- **Package Manager:** pacman + AUR support
- **Init:** systemd
- **Build System:** archiso (ISO generation)

## Project Structure

```
aurora-os/
├── branding/          # Logos, boot splash assets
├── build-system/      # archiso profile & build scripts
├── desktop-environment/
│   ├── compositor/    # Wayland compositor (wlroots)
│   ├── panel/         # Top/bottom panel (taskbar)
│   ├── widgets/       # Desktop widgets
│   ├── app-launcher/  # Application launcher
│   └── settings/      # Settings application
├── themes/
│   ├── gtk/           # Aurora GTK theme
│   ├── icons/         # Aurora icon pack
│   ├── cursors/       # Aurora cursor theme
│   ├── wallpapers/    # Default wallpapers
│   └── plymouth/      # Boot splash theme
├── installer/         # Custom installer (calamares config)
└── docs/              # Documentation
```

## Building

```bash
# Build the ISO
cd build-system
sudo ./build.sh
```

## License

TBD
