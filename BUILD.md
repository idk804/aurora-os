# Building Aurora OS

This guide covers multiple ways to build the Aurora OS ISO.

---

## Option 1: Docker (Recommended for WSL/macOS/Linux)

### 1. Install Docker

**WSL (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install docker.io
sudo service docker start
sudo usermod -aG docker $USER
# Log out and back in
```

**macOS:**
```bash
brew install --cask docker
# Start Docker Desktop
```

**Linux:**
```bash
sudo apt install docker.io
sudo systemctl enable docker
sudo systemctl start docker
sudo usermod -aG docker $USER
```

### 2. Clone and Build

```bash
git clone https://github.com/idk804/aurora-os.git
cd aurora-os/build-system

# Build the Docker image
docker build -t aurora-build .

# Run the build (this will take a while)
docker run --rm -v $(pwd)/out:/build/out aurora-build ./build.sh
```

### 3. Find Your ISO

The built ISO will be in:
```
build-system/out/aurora-os-*.iso
```

---

## Option 2: Native Arch Linux

### Requirements

- Arch Linux (or Arch-based distro like Manjaro, EndeavourOS)
- ~20GB free disk space
- Root access (sudo)

### Build Steps

```bash
# 1. Install dependencies
sudo pacman -Syu archiso git base-devel

# 2. Clone repo
git clone https://github.com/idk804/aurora-os.git
cd aurora-os

# 3. Build
cd build-system
sudo ./build.sh
```

The ISO will be at `build-system/out/aurora-os-*.iso`

---

## Option 3: Virtual Machine

If Docker doesn't work on your system, use a VM:

1. **Download Arch Linux ISO:**
   https://archlinux.org/download/

2. **Create VM:**
   - VMware Workstation/Fusion
   - VirtualBox
   - Hyper-V

3. **In the VM:**
   - Install Arch Linux
   - Run: `sudo pacman -Syu archiso git base-devel`
   - Clone and run build script

---

## Troubleshooting

### Docker build fails with "permission denied"
```bash
sudo service docker start
sudo usermod -aG docker $USER
# Log out and back in
```

### "archiso not found"
```bash
# On Arch/Manjaro
sudo pacman -S archiso

# On Debian/Ubuntu (for Docker)
# Already in Dockerfile, should work
```

### Out of disk space
```bash
# Clean Docker
docker system prune -a

# Or increase WSL disk size
wsl --shutdown
diskpart
(virtual disk max size)
```

---

## What's Inside the Build

The ISO includes:
- Arch Linux base system
- Aurora Compositor (wlroots Wayland compositor)
- Aurora Desktop Environment (panel, launcher, widgets, settings)
- Aurora GTK themes (dark mode with aurora accents)
- Plymouth boot splash
- Calamares installer

---

## Next Steps

After building the ISO:
1. Flash to USB: `sudo dd if=aurora-os.iso of=/dev/sdX`
2. Boot from USB
3. Install via Calamares installer

---

Questions? Open an issue: https://github.com/idk804/aurora-os/issues