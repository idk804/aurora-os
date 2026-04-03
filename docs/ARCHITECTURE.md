# Aurora OS Architecture

## System Stack

```
┌─────────────────────────────────────────┐
│           Aurora Applications           │
│  (Settings, File Manager, Terminal...)  │
├─────────────────────────────────────────┤
│          Aurora Desktop Shell           │
│  Panel │ Widgets │ App Launcher │ Notif │
├─────────────────────────────────────────┤
│         Aurora Compositor (ADE)         │
│         (wlroots-based Wayland)         │
├─────────────────────────────────────────┤
│              Linux Kernel               │
│           (Arch Linux base)             │
└─────────────────────────────────────────┘
```

## Component Breakdown

### 1. Base System (Arch Linux)
- Rolling release model
- pacman package manager
- systemd init system
- Linux kernel (latest stable)

### 2. Aurora Compositor
- Built on **wlroots** library
- Wayland-native (no X11 dependency for core, XWayland optional)
- Window management: tiling + floating hybrid
- Animations: smooth, aurora-inspired transitions
- Multi-monitor support

### 3. Aurora Panel
- Top bar: clock, system tray, notifications, quick settings
- Bottom dock: favorites, running apps, workspace switcher
- Translucent with aurora glow effects

### 4. Aurora Widgets
- Desktop overlay widgets
- Clock with aurora gradient
- Weather with animated backgrounds
- System monitor (CPU, RAM, disk)
- Media player controls
- Configurable layout (drag & drop)

### 5. Aurora App Launcher
- Full-screen or compact mode
- Fuzzy search
- Categorized app grid
- Recent apps & files
- Aurora blur background

### 6. Aurora Theme
- **Color Palette:**
  - Background: Deep navy/black (#0a0a1a, #0d1117)
  - Primary: Aurora green (#00ff87, #39ff14)
  - Secondary: Aurora purple (#b388ff, #7c4dff)
  - Accent: Aurora cyan (#00e5ff, #18ffff)
  - Text: Soft white (#e0e0e0)
- **GTK Theme:** Dark with aurora accent colors
- **Icons:** Minimal, line-based with gradient fills
- **Cursors:** Sleek with subtle glow
- **Plymouth:** Animated aurora borealis boot splash

### 7. Installer
- Based on Calamares
- Custom Aurora branding
- Guided partitioning
- User setup with avatar selection

## Boot Sequence

1. BIOS/UEFI → GRUB (Aurora branded)
2. Plymouth splash (aurora animation)
3. systemd → Display manager (Aurora greeter)
4. Aurora Compositor + Desktop Shell
