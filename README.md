# iamroot

[![Packaging status](https://repology.org/badge/vertical-allrepos/iamroot.svg)](https://repology.org/project/iamroot/versions)
[![CodeQL](https://github.com/gportay/iamroot/actions/workflows/codeql.yml/badge.svg)](https://github.com/gportay/iamroot/actions/workflows/codeql.yml)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/b167e3d1271545d2b6e1a416bcf3d00d)](https://app.codacy.com/gh/gportay/iamroot?utm_source=github.com&utm_medium=referral&utm_content=gportay/iamroot&utm_campaign=Badge_Grade_Settings)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/db94176b5c354fe7bbdf93a194064140)](https://www.codacy.com/gh/gportay/iamroot/dashboard?utm_source=github.com&utm_medium=referral&utm_content=gportay/iamroot&utm_campaign=Badge_Coverage)
[![FreeBSD-vm](https://github.com/gportay/iamroot/actions/workflows/FreeBSD-vm.yml/badge.svg)](https://github.com/gportay/iamroot/actions/workflows/FreeBSD-vm.yml)
[![OpenBSD-vm](https://github.com/gportay/iamroot/actions/workflows/OpenBSD-vm.yml/badge.svg)](https://github.com/gportay/iamroot/actions/workflows/OpenBSD-vm.yml)
[![NetBSD-vm](https://github.com/gportay/iamroot/actions/workflows/NetBSD-vm.yml/badge.svg)](https://github.com/gportay/iamroot/actions/workflows/NetBSD-vm.yml)

[iamroot(7)] emulates privileged syscalls such as [chroot(2)] for unprivileged
processes in userspace.

## TL;DR;

[iamroot(7)] provides a self-contained and an all-in-one alternative to both
[fakeroot(1)] and [fakechroot(1)].

The project targets the *Linux* userlands [glibc] and [musl]. However, it works
on [FreeBSD] (13.1), [OpenBSD] (7.2 and 7.3) and [NetBSD] (9.3) even if its
usage is limited by some statically linked binaries (such as `pkg-static`,
`chroot`...) and by none-executable dynamic loaders.

The project compiles on Intel x86 and ARM 64-bit, and it runs on [Arch Linux],
[Debian], [Alpine Linux], [FreeBSD] (13.1), [OpenBSD] (7.2 and 7.3) and
[NetBSD] (9.3).

The [Miscellaneous Binary Format][binfmt_misc] on *Linux* allows to [chroot(2)]
in a rootfs directory using a different architecture thanks to emulators (such
as the [QEMU user-mode emulation][qemu] static binaries). The architectures
`x86_64`, `i386`, `aarch64`, `armhf`, `arm`, and `riscv64` are supported.

## HOW IT WORKS

It consists of an ELF shim library which is preloaded using the environment
variable `LD_PRELOAD`. It intercepts the calls to the libc functions with a
`filename` or `pathname` in parameter ([open(2)], [fopen(3)], [stat(2)],
[readlink(2)], [chown(2)], [chdir(2)], [chroot(2)]...).

The syscall [chroot(2)] changes a small ingredient in the pathname resolution
process; it is visible via each process's symlink `/proc/self/root`. The
environment variable `IAMROOT_ROOT` implements that behaviour in the world of
[iamroot(7)]. Basically, it replaces the leading `/` of an absolute pathname
with the alternate root.

For the curious, the magic operates in files [chroot.c](chroot.c) for entering
in chroot, in [chdir.c](chdir.c) and [fchdir.c](fchdir.c) for exiting "chroot
jail", in [path_resolution.c](path_resolution.c) for resolving pathnames, and
in [execve.c](execve.c) and [dso.c](dso.c) for exec'ing executable files from
chroot.

Of course, [iamroot(7)] cannot substitute itself to the superuser permissions,
and commands will end with `EACCESS` or `EPERM` as of reading or writing files
in `/proc`, `/sys`, `/dev` or `/run`, to name but a few.

## BUILD ROOTFS

[iamroot(7)] aims to create any Linux rootfs using the package manager of the
distribution (or its bootstrap script).

The table below lists the distributions and its tool that work with.

| Tool                 | Distributions                                        |
| -------------------- | ---------------------------------------------------- |
| [pacstrap(8)]        | Arch Linux, Arch Linux ARM, Arch Linux 32, Manjaro   |
| [alpine-make-rootfs] | Alpine Linux                                         |
| [dnf(8)]             | Fedora                                               |
| [zypper(8)]          | openSUSE                                             |
| [debootstrap(8)]     | Debian\*, Ubuntu\*, Devuan\*                         |
| [xbps-install(1)]    | Void Linux                                           |

\*: Works with hacks.

## FAKECHROOT

[fakechroot(1)] is perfectible to create rootfs. There are several issues to get addressed.

[fakechroot(1)] does not strip de chroot directory in the absolute symlink targets and, consequently, leaks the chroot directory in the alternate rootfs. Either, it does not resolves and follows the symlinks correctly.

[dnf(8)] enters and exits the "chroot jail" to run the packages scriptlets but [fakechroot(1)] does not support exiting if [chdir(2)]'ing out of the chroot directory. Moreover, the function [fchdir(2)] is not intercepted.

## DOCUMENTATION

Build the documentation using *make(1)*

	$ make doc
	asciidoctor -b manpage -o ish.1 ish.1.adoc
	gzip -c ish.1 >ish.1.gz
	asciidoctor -b manpage -o iamroot.7 iamroot.7.adoc
	gzip -c iamroot.7 >iamroot.7.gz
	rm iamroot.7 ish.1

## BUILD

Run the following command to build *libiamroot.so*

For your home directory (i.e. your user only)

	$ make libiamroot.so PREFIX=$HOME/.local

Or, for your system (i.e. every users)

	$ make libiamroot.so

## INSTALL

Run the following command to install *iamroot(7)* and *ish(1)*

To your home directory (i.e. your user only)

	$ make user-install

Or, to your system (i.e. every users)

	$ sudo make install

The traditional variables *DESTDIR* and *PREFIX* can be overridden

	$ sudo make install PREFIX=/opt/iamroot

Or

	$ make install DESTDIR=$PWD/pkg PREFIX=/usr

## BUGS

Report bugs at *https://github.com/gportay/iamroot/issues*

# AUTHOR

Written by Gaël PORTAY *gael.portay@gmail.com*

## COPYRIGHT

Copyright (c) 2021-2024 Gaël PORTAY

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 2.1 of the License, or (at your option) any
later version.

## SEE ALSO

[iamroot(7)], [ish(1)], [chroot(2)], [path_resolution(7)], [fakechroot(1)],
[fakeroot(1)], [binfmt_misc], [qemu]

[Alpine Linux]: https://www.alpinelinux.org/
[Arch Linux]: https://archlinux.org/
[Debian]: https://www.debian.org/
[FreeBSD]: https://www.freebsd.org/
[NetBSD]: https://www.netbsd.org/
[OpenBSD]: https://www.openbsd.org/
[alpine-make-rootfs]: https://github.com/alpinelinux/alpine-make-rootfs/blob/master/README.adoc
[binfmt_misc]: https://www.kernel.org/doc/html/latest/admin-guide/binfmt-misc.html
[chdir(2)]: https://linux.die.net/man/2/chdir
[chown(2)]: https://linux.die.net/man/2/chown
[chroot(2)]: https://linux.die.net/man/2/chroot
[debootstrap(8)]: https://linux.die.net/man/8/debootstrap
[dnf(8)]: https://dnf.readthedocs.io/en/latest/command_ref.html
[fakechroot(1)]: https://linux.die.net/man/1/fakechroot
[fakeroot(1)]: https://linux.die.net/man/1/fakeroot-sysv
[fchdir(2)]: https://linux.die.net/man/2/fchdir
[fopen(3)]: https://linux.die.net/man/3/fopen
[glibc]: https://www.gnu.org/software/libc/
[iamroot(7)]: iamroot.7.adoc
[ish(1)]: ish.1.adoc
[musl]: https://www.musl-libc.org/
[open(2)]: https://linux.die.net/man/2/open
[pacstrap(8)]: https://man.archlinux.org/man/pacstrap.8
[path_resolution(7)]: https://linux.die.net/man/7/path_resolution
[qemu]: https://www.qemu.org/
[readlink(2)]: https://linux.die.net/man/2/readlink
[stat(2)]: https://linux.die.net/man/2/stat
[xbps-install(1)]: https://man.voidlinux.org/xbps-install.1
[zypper(8)]: https://en.opensuse.org/SDB:Zypper_manual_(plain)
