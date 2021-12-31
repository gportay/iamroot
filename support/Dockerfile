# syntax=docker/dockerfile:1
FROM archlinux/archlinux
RUN pacman -Syu --noconfirm base-devel sudo
RUN pacman -Syu --noconfirm aarch64-linux-gnu-gcc asciidoctor git kernel-headers-musl musl shellcheck
RUN pacman -Syu --noconfirm arch-install-scripts
