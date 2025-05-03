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

[iamroot(7)] works towards creating rootfs from the binary packages of the most
popular *Linux* distributions without the need for [sudo(8)].

The project is a self-contained and all-in-one alternative to [fakeroot(1)] and
[fakechroot(1)]. It gives unprivileged user permissions to call privileged
syscalls such as [chown(2)] and [chroot(2)] by emulating them.

[iamroot(7)] targets the *Linux* userlands [glibc] and [musl]. It works on
[FreeBSD], [OpenBSD] and [NetBSD] even if its usage is limited by various
statically linked binaries and by non-executable dynamic loaders.

The project compiles on Intel x86 and ARM 64-bit, and it runs on [Arch Linux],
[Debian], [Alpine Linux], [FreeBSD], [OpenBSD] and [NetBSD].

The [Miscellaneous Binary Format][binfmt_misc] on *Linux* allows to [chroot(2)]
in a rootfs directory using a different architecture thanks to emulators (such
as the [QEMU user-mode emulation][qemu] static binaries). The architectures
`x86_64`, `i386`, `aarch64`, `aarch64_be`, `armhf`, `arm`, `riscv64`, `mipsle`
`mips64le`, `powerpc64`, `powerpc64le`, `powerpc` and `s390x` are supported.

## HOW IT WORKS

### PRELOAD

It consists of an ELF shim library which is preloaded using the environment
variable `LD_PRELOAD`.

It intercepts the calls to the libc functions requiring privileged syscalls or
having a `filename` or `pathname` in parameter. Some functions re-implement the
functionality, somes are simple stubs returning success.

### ROOT DIRECTORY

The syscall [chroot(2)] changes a small ingredient in the pathname resolution
process; it is visible via each process's symlink `/proc/self/root`. The
environment variable `IAMROOT_ROOT` implements that behaviour in the world of
[iamroot(7)]. Basically, it replaces the leading `/` of an absolute pathname
with the alternate root directory path.

### MAGIC

For the curious, the magic operates in files [chroot.c](chroot.c) for entering
in chroot, in [chdir.c](chdir.c) and [fchdir.c](fchdir.c) for exiting "chroot
jail", in [path_resolution.c](path_resolution.c) for resolving pathnames, and
in [execve.c](execve.c) and [dso.c](dso.c) for exec'ing executable files from
chroot by rewriting the whole command-line using the [dynamic loader][ld.so(8)]
and it's options (i.e. `--preload`, `--library-path` and `--argv0` if they
exist).

### PERMISSION DENIED

Of course, [iamroot(7)] cannot substitute itself to the superuser permissions,
and commands will end with `EACCESS` or `EPERM` as of reading or writing files
in `/proc`, `/sys`, `/dev` or `/run`.

### HELPER SCRIPTS

[iamroot(7)] is configurable via environment variables to overcome specific
situations of the whole variety of operation-systems (i.e. GNU/Linux, musl,
\*BSD...).

It comes with two shell scripts to make the configuration easy via specific
command line options. They are frontends to standard CLIs to either open an
interactive shell or to switch user. [ish(1)] provides a shell like [sh(1)],
and [ido(1)] switches user like [sudo(8)].

## BUILD ROOTFS

[iamroot(7)] aims to create any *Linux* rootfs using the package manager of the
distribution (or its bootstrap script).

The table below lists the distributions and its tool that work with.

| Tool                 | Distributions                                                                                 |
| -------------------- | --------------------------------------------------------------------------------------------- |
| [pacstrap(8)]        | [Arch Linux], [Arch Linux ARM], [Arch Linux 32], [Arch Linux RISC-V], [Arch POWER], [Manjaro] |
| [alpine-make-rootfs] | [Alpine Linux], [Adélie Linux]                                                                |
| [dnf(8)]             | [Fedora]                                                                                      |
| [zypper(8)]          | [openSUSE]                                                                                    |
| [debootstrap(8)]     | [Debian], [Ubuntu]\*, [Devuan]                                                                |
| [mmdebstrap(1)]      | [Mobian], [Raspbian]                                                                          |
| [xbps-install(1)]    | [Void Linux]                                                                                  |

\*: Works with hacks.

## GLIBC

The [GNU C Library][glibc] leaks symbols in the dynamically linked binaries and
shared libraries.

As a consequence, the binaries or the shared libraries need to run along with
the same or any later version of the [glibc].

For this reason, the [iamroot(7)] library is **NOT** linked against the
[default libraries][-nodefaultlibs] by the [dynamic linker][ld(1)] at
link-time.

Instead, the libraries `libc.so.6`, `libdl.so.2`, `libpthread.so.0` and
`libgcc_s.so.1` are `patchelf`'ed manually afterward.

This works around the missing symbols raised by the [dynamic loader][ld.so(8)]
at run-time.

However, it does not solve for the missing symbols leaked in the binaries and
shared libraries loaded from within the [chroot(2)]-ed directory.

The symbols must be implemented in the [iamroot(7)] library or loaded from a
third-party library (such as the [gcompat] library which runs [glibc]-binaries
from a [musl]-system).

## FAKECHROOT

[fakechroot(1)] is perfectible to create rootfs. There are several issues to
address to make a rootfs.

[fakechroot(1)] does not strip de chroot directory in the absolute symlink
targets and, consequently, leaks the chroot directory in the alternate rootfs.
Either, it does not resolves and follows the symlinks correctly.

[dnf(8)] enters and exits the "chroot jail" to run the packages scriptlets but
[fakechroot(1)] does not support exiting if [chdir(2)]'ing out of the chroot
directory. Moreover, the function [fchdir(2)] is not intercepted.

Besides, the [GNU C Library][glibc] leaks symbols in the dynamically linked
binaries. Therefore, the binaries *MUST* load the same or any later version of
the libc. [fakechroot(1)] runs the host wide dynamic loader, hence, it cannot
run binaries or load libraries linked against newer version of glibc with extra
new symbols; even setting the environment variable `FAKECHROOT_ELFLOADER` or
using the option `--use-system-libs`. And this includes the library
`libfakechroot.so` itself.

It is even worse if considering running binaries for another architecture, as
the appropriate dynamic loader *MUST* get installed on the host system to get
run by the kernel. Additionnaly, the library `libfakechroot.so` *MUST* get
installed alongside the rootfs. However, [fakechroot(1)] does not compile in a
[musl] world.

[fakechroot(1)] blindly trusts the environment variable `LD_PRELOAD` is set and
preserved across the processes, although, [dracut(8)] unset the dynamic loader
environment variables and it shoots itself in the foot.

For all those latter reasons and since the *Linux* [ld.so(8)] can execute
binaries, then [iamroot(7)] rewrites the whole command line to overcome all the
limitations mentioned above.

## PSEUDO

[pseudo(1)] is another alternative to both [fakeroot(1)] and [fakechroot(1)].
It is used by [The Yocto Project].

It appears [pseudo(1)] is compiled and linked against the host libc,
accordingly, [pseudo(1)] would fail to [chroot(2)] and run a binary linked
against former versions of glibc with missing new symbols.

## DOCUMENTATION

Build the documentation using *make(1)*

	$ make doc
	asciidoctor -b manpage -o ido.1 ido.1.adoc
	gzip -c ido.1 >ido.1.gz
	asciidoctor -b manpage -o ish.1 ish.1.adoc
	gzip -c ish.1 >ish.1.gz
	asciidoctor -b manpage -o iamroot.7 iamroot.7.adoc
	gzip -c iamroot.7 >iamroot.7.gz
	asciidoctor -b manpage -o ld-iamroot.so.8 ld-iamroot.so.8.adoc
	gzip -c ld-iamroot.so.8 >ld-iamroot.so.8.gz
	rm iamroot.7 ish.1 ido.1 ld-iamroot.so.8

## BUILD

Run the following command to build *ld-iamroot.so* and *libiamroot.so*

For your home directory (i.e. your user only)

	$ make ld-iamroot.so libiamroot.so PREFIX=$HOME/.local

Or, for your system (i.e. every users)

	$ make ld-iamroot.so libiamroot.so

## INSTALL

Run the following command to install *iamroot(7)*, *ido(1)* and *ish(1)*

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

Copyright (c) 2021-2025 Gaël PORTAY

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 2.1 of the License, or (at your option) any
later version.

## SEE ALSO

[iamroot(7)], [ido(1)], [ish(1)], [ld-iamroot.so(8)], [chroot(2)],
[path_resolution(7)], [fakechroot(1)], [fakeroot(1)], [pseudo(1)],
[binfmt_misc], [qemu]

[-nodefaultlibs]: https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html#index-nodefaultlibs
[Adélie Linux]: https://www.adelielinux.org/
[Alpine Linux]: https://www.alpinelinux.org/
[Alpine Linux]: https://www.alpinelinux.org/
[Arch Linux 32]: https://archlinux32.org/
[Arch Linux ARM]: https://archlinuxarm.org/
[Arch Linux RISC-V]: https://archriscv.felixc.at/
[Arch Linux]: https://archlinux.org/
[Arch POWER]: https://archlinuxpower.org/
[Debian]: https://www.debian.org/
[Devuan]: https://www.devuan.org/
[Fedora]: https://fedoraproject.org/
[FreeBSD]: https://www.freebsd.org/
[Manjaro]: https://manjaro.org/
[Mobian]: https://mobian-project.org/
[NetBSD]: https://www.netbsd.org/
[OpenBSD]: https://www.openbsd.org/
[Raspbian]: https://www.raspberrypi.com/software/
[The Yocto Project]: https://www.yoctoproject.org/software-item/poky/
[Ubuntu]: https://ubuntu.com/
[Void Linux]: https://voidlinux.org/
[alpine-make-rootfs]: https://github.com/alpinelinux/alpine-make-rootfs/blob/master/README.adoc
[binfmt_misc]: https://www.kernel.org/doc/html/latest/admin-guide/binfmt-misc.html
[chdir(2)]: https://linux.die.net/man/2/chdir
[chown(2)]: https://linux.die.net/man/2/chown
[chroot(2)]: https://linux.die.net/man/2/chroot
[debconf]: https://packages.debian.org/sid/debconf
[debootstrap(8)]: https://linux.die.net/man/8/debootstrap
[dnf(8)]: https://dnf.readthedocs.io/en/latest/command_ref.html
[dracut(8)]: https://linux.die.net/man/8/dracut
[fakechroot(1)]: https://linux.die.net/man/1/fakechroot
[fakeroot(1)]: https://linux.die.net/man/1/fakeroot-sysv
[fchdir(2)]: https://linux.die.net/man/2/fchdir
[gcompat]: https://gcompat.org/
[glibc]: https://www.gnu.org/software/libc/
[iamroot(7)]: iamroot.7.adoc
[ido(1)]: ido.1.adoc
[ish(1)]: ish.1.adoc
[ld(1)]: https://linux.die.net/man/1/ld
[ld-iamroot.so(8)]: ld-iamroot.so.8.adoc
[ld.so(8)]: https://linux.die.net/man/8/ld.so
[libpam-runtime]: https://packages.debian.org/sid/libpam-runtime
[mmdebstrap(1)]: https://manpages.debian.org/testing/mmdebstrap/mmdebstrap.1.en.html
[musl]: https://www.musl-libc.org/
[musl]: https://www.musl-libc.org/
[open(2)]: https://linux.die.net/man/2/open
[openSUSE]: https://www.opensuse.org/
[pacstrap(8)]: https://man.archlinux.org/man/pacstrap.8
[path_resolution(7)]: https://linux.die.net/man/7/path_resolution
[pseudo(1)]: https://git.yoctoproject.org/pseudo
[qemu]: https://www.qemu.org/
[sh(1)]: https://linux.die.net/man/1/sh
[sudo(8)]: https://linux.die.net/man/8/sudo
[xbps-install(1)]: https://man.voidlinux.org/xbps-install.1
[zypper(8)]: https://en.opensuse.org/SDB:Zypper_manual_(plain)
