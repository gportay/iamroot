# syntax=docker/dockerfile:1
# Requires the docker build options:
# --build-arg "user=$USER" --build-arg "uid=$UID" --build-arg "groups=$GROUPS" --build-arg "home=$HOME"
FROM archlinux/archlinux:latest
RUN pacman -Syu --noconfirm base-devel sudo
RUN pacman -Syu --noconfirm asciidoctor git kernel-headers-musl lib32-glibc musl patchelf shellcheck
RUN pacman -Syu --noconfirm arch-install-scripts

ARG user
ARG uid
ARG groups
ARG home

RUN groupadd --non-unique --gid "$groups" "$user"
RUN useradd  --non-unique --gid "$groups" --uid "$uid" --create-home --home-dir "$home" --shell "$SHELL" "$user"
RUN echo "%$user ALL=(ALL) NOPASSWD: /usr/bin/pacman" >"/etc/sudoers.d/$user"

ENV VISUAL cat
ENV EDITOR cat
USER $user
WORKDIR $home

RUN echo 'source /etc/makepkg.conf' >~/.makepkg.conf
RUN echo 'MAKEFLAGS="-j$(nproc)"' >>~/.makepkg.conf
RUN echo 'OPTIONS+=(!strip)' >>~/.makepkg.conf
RUN mkdir -p ~/.cache/pacaur

RUN git clone https://aur.archlinux.org/auracle-git.git "$home/.cache/pacaur/auracle-git"
WORKDIR $home/.cache/pacaur/auracle-git
RUN makepkg --syncdeps --install --noconfirm

RUN git clone https://aur.archlinux.org/pacaur.git "$home/.cache/pacaur/pacaur"
WORKDIR $home/.cache/pacaur/pacaur
RUN makepkg --syncdeps --install --noconfirm

RUN gpg --recv-keys 56BCDB593020450F
RUN pacaur -Syu --noconfirm i386-musl kernel-headers-musl-i386 
RUN pacaur -Syu --noconfirm aarch64-glibc-bleeding-edge-toolchain aarch64-musl-bleeding-edge-toolchain
RUN pacaur -Syu --noconfirm armv7-eabihf-glibc-bleeding-edge-toolchain armv7-eabihf-musl-bleeding-edge-toolchain
RUN pacaur -Syu --noconfirm armv5-eabi-glibc-bleeding-edge-toolchain armv5-eabi-musl-bleeding-edge-toolchain
RUN pacaur -Syu --noconfirm riscv64-lp64d-glibc-bleeding-edge-toolchain riscv64-lp64d-musl-bleeding-edge-toolchain
