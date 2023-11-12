# syntax=docker/dockerfile:1
# Requires the docker build options:
# --build-arg "user=$USER" --build-arg "uid=$UID" --build-arg "groups=$GROUPS" --build-arg "home=$HOME"
FROM archlinux/archlinux:latest
RUN pacman -Syu --noconfirm base-devel sudo
RUN pacman -Syu --noconfirm aarch64-linux-gnu-gcc asciidoctor git kernel-headers-musl lib32-glibc musl patchelf shellcheck
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

RUN gpg --recv-keys 13FCEF89DD9E3C4F 16792B4EA25340F8 6C35B99309B5FA62 38DBBDC86092693E
RUN CFLAGS="-Wno-error=format-security" pacaur -Syu --noconfirm arm-linux-gnueabihf-gcc
RUN pacaur -Syu --noconfirm muslcc-arm-linux-musleabihf-cross-bin
ENV PATH /opt/muslcc/arm-linux-musleabihf-cross/bin:$PATH
RUN gpg --recv-keys 56BCDB593020450F
RUN pacaur -Syu --noconfirm i386-musl kernel-headers-musl-i386 armhf-musl kernel-headers-musl-armhf aarch64-linux-musl kernel-headers-musl-aarch64
