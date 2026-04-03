# Aurora OS Docker Build Environment
# Build Aurora OS ISO on any Linux (WSL, macOS, Debian, etc.)

FROM archlinux:latest

RUN pacman -Sy --noconfirm \
    archiso \
    git \
    base-devel \
    sudo \
    which \
    curl \
    wget

WORKDIR /build

COPY . .

RUN chmod +x build-system/build.sh

CMD ["/bin/bash"]