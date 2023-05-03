# iamroot

[![Packaging status](https://repology.org/badge/vertical-allrepos/iamroot.svg)](https://repology.org/project/iamroot/versions)
[![CodeQL](https://github.com/gportay/iamroot/actions/workflows/codeql.yml/badge.svg)](https://github.com/gportay/iamroot/actions/workflows/codeql.yml)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/b167e3d1271545d2b6e1a416bcf3d00d)](https://app.codacy.com/gh/gportay/iamroot?utm_source=github.com&utm_medium=referral&utm_content=gportay/iamroot&utm_campaign=Badge_Grade_Settings)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/db94176b5c354fe7bbdf93a194064140)](https://www.codacy.com/gh/gportay/iamroot/dashboard?utm_source=github.com&utm_medium=referral&utm_content=gportay/iamroot&utm_campaign=Badge_Coverage)
[![FreeBSD-vm](https://github.com/gportay/iamroot/actions/workflows/FreeBSD-vm.yml/badge.svg)](https://github.com/gportay/iamroot/actions/workflows/FreeBSD-vm.yml)

_Disclaimer_: The project is still in development; therefore it is not suitable
for a production usage. Consult the [changelog] for the recent changes. The
project is not mature yet, and the API are subject to change accross the
versions with no backward compatibility.

## TL;DR;

[iamroot(7)] emulates the syscall [chroot(2)] for unprivileged processes in
userspace.

It aims to provide a self-contained and all-in-one alternative to [fakeroot(1)]
and [fakechroot(1)].

The project targets the *Linux* userlands [glibc] and [musl]; the works to
support *BSD* userlands such as [FreeBSD] (13.1) or [OpenBSD] is on-going.

The project compiles on x86 and ARM 64-bit, and runs on [Arch Linux], [Debian]
and [FreeBSD] 13.1.

## HOW IT WORKS

It consists of an ELF shim library which is preloaded using the environment
variable `LD_PRELOAD` and which intercepts the calls to libc functions with a
`filename` or `pathname` in parameter ([open(2)], [fopen(3)], [stat(2)],
[readlink(2)], [chown(2)], [chdir(2)], [chroot(2)]...).

The syscall [chroot(2)] changes a small ingredient in the pathname resolution
process; it is visible via each process's symlink `/proc/self/root`. The
environment variable `IAMROOT_ROOT` implements the behaviour in the world of
iamroot. Basically, it replaces the leading `/` of an absolute pathname with
the alternate root.

For the curious, the magic operates in files [chroot.c](chroot.c) for entering
in chroot, in [chdir.c](chdir.c) and [fchdir.c](fchdir.c) for exiting "chroot
jail", in [path_resolution.c](path_resolution.c) for resolving pathnames, and
in [execve.c](execve.c) for exec'ing executable form chroot.

[fakechroot(1)] does not run well for creating rootfs with [pacstrap(8)] (Arch
Linux, Manjaro), [alpine-make-rootfs] (Alpine Linux), [dnf(8)] (Fedora),
[zypper(8)] (openSUSE) or [debootstrap(8)] (Debian, Ubuntu).  Its existing
world would likely break if it is hacked to address the rootfs-creation related
issues (i.e. fixing entering-exiting chroot and absolute symlink resolution in
short).

Of course, iamroot cannot substitute to the superuser permissions, and commands
will end with `EACCESS` or `EPERM` as of reading or writing files in `/proc`,
`/sys`, `/dev` or `/run`, to name but a few.

iamroot is a proof-of-concept, therefore expect the unexpectable!

# BUGS

Report bugs at *https://github.com/gportay/iamroot/issues*

# AUTHOR

Written by Gaël PORTAY *gael.portay@gmail.com*

# COPYRIGHT

Copyright (c) 2021-2023 Gaël PORTAY

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 2.1 of the License, or (at your option) any
later version.

# SEE ALSO

[iamroot(7)], [iamroot-shell(1)], [chroot(2)], [path_resolution(7)],
[fakechroot(1)]

[Arch Linux]: https://archlinux.org/
[Debian]: https://www.debian.org/
[FreeBSD]: https://www.freebsd.org/
[OpenBSD]: https://www.openbsd.org/
[alpine-make-rootfs]: https://github.com/alpinelinux/alpine-make-rootfs/blob/master/README.adoc
[changelog]: CHANGELOG.md#unreleased
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
[iamroot-shell(1)]: iamroot-shell.1.adoc
[musl]: https://www.musl-libc.org/
[open(2)]: https://linux.die.net/man/2/open
[pacstrap(8)]: https://man.archlinux.org/man/pacstrap.8
[path_resolution(7)]: https://linux.die.net/man/7/path_resolution
[readlink(2)]: https://linux.die.net/man/2/readlink
[stat(2)]: https://linux.die.net/man/2/stat
[zypper(8)]: https://en.opensuse.org/SDB:Zypper_manual_(plain)
