# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [unreleased]

## [v2] - 2021-10-01

### Added

- Run the [dynamic linker][ld.so(8)] directly
- Chroot to a [musl] world
- Create an [Alpine Linux] rootfs via [alpine-make-rootfs]
- Intercept the [SUID][capabilities(7)] commands [passwd(1)] and [su(1)] in
  `exec.sh`
- Intercept the [glibc] internal function [__nss_files_fopen()] to not ignore
  anymore the command [systemd-sysusers(8)] in `IAMROOT_EXEC_IGNORE` and to not
  intercept it anymore in `exec.sh`
- Output the [exec'ed][exec(3)] commands if `IAMROOT_EXEC_DEBUG` is set
- Output the unhandled commands even if `IAMROOT_DEBUG` is unset

### Fixed

- Handle properly [busybox(1)] if used in an hashbang

## [v1] - 2021-04-14

Initial release.

[Alpine Linux]: https://alpinelinux.org/
[__nss_files_fopen()]: https://sourceware.org/git/?p=glibc.git;a=blob;f=nss/nss_files_fopen.c;h=594e4216578766e4534c44dc6c75283d5d1a20fe;hb=299210c1fa67e2dfb564475986fce11cd33db9ad
[alpine-make-rootfs]: https://github.com/alpinelinux/alpine-make-rootfs/blob/master/README.adoc
[busybox(1)]: https://linux.die.net/man/1/busybox
[capabilities(7)]: https://linux.die.net/man/7/capabilities
[exec(3)]: https://linux.die.net/man/3/exec
[glibc]: https://www.gnu.org/software/libc/
[ld.so(8)]: https://linux.die.net/man/8/ld.so
[musl]: https://www.musl-libc.org/
[passwd(1)]: https://linux.die.net/man/1/passwd
[su(1)]: https://linux.die.net/man/1/su
[systemd-sysusers(8)]: https://www.freedesktop.org/software/systemd/man/systemd-sysusers.html
[unreleased]: https://github.com/gportay/iamroot/compare/v2...master
[v1]: https://github.com/gportay/iamroot/releases/tag/v1
[v2]: https://github.com/gportay/iamroot/compare/v1...v2
