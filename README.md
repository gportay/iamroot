# iamroot

## TL;DR;

[iamroot(7)] emulates the syscall [chroot(2)] for unprivileged processes in
userspace.

## HOW IT WORKS

It consists of an ELF library which is preloaded using the environment variable
`LD_PRELOAD` and which intercepts the calls to libc functions with a `filename`
or `pathname` in parameter ([open(2)], [fopen(3)], [stat(2)], [readlink(2)],
[chown(2)], [chdir(2)], [chroot(2)]...).

The syscall [chroot(2)] changes a small ingredient in the pathname resolution
process; it is visible via each process's symlink `/proc/self/root`. The
environment variable `IAMROOT_ROOT` implements the behaviour in the world of
iamroot. Basically, it replaces the leading `/` of an absolute pathname with
the alternate root.

For the curious, the magic operates in files [chroot.c](chroot.c) for entering
in chroot, in [chdir.c](chdir.c) and [fchdir.c](fchdir.c) for exiting "chroot
jail", in [path_resolution.c](path_resolution.c) for resolving pathnames, and
in [execve.c](execve.c) for exec'ing executable form chroot.

iamroot aims to provide an alternative to [fakechroot(1)], which does great but
does not run well for creating rootfs with [pacstrap(8)] (Arch Linux),
[alpine-make-rootfs] (Alpine Linux), or [dnf(8)] (Fedora). Its existing world
would likely break if it is hacked to address the rootfs-creation related
issues (i.e. fixing entering-exiting chroot and absolute symlink resolution in
short).

Of course, iamroot cannot substitute to the superuser permissions, and commands
will end with `EACCESS` or `EPERM` as of reading or writing files in `/proc`,
`/sys`, `/dev` or `/run`, to name but a few.

iamroot is a proof-of-concept, therefore expect the unexpectable!

## CHROOT(2)

The man page of [chroot(2)] tells:

> chroot() changes the root directory of the calling process to that specified
> in path. This directory will be used for pathnames beginning with /. The root
> directory is inherited by all children of the calling process.
>
> Only a privileged process (Linux: one with the CAP_SYS_CHROOT capability in
> its user namespace) may call chroot().
>
> This call changes an ingredient in the pathname resolution process and does
> nothing else. In particular, it is not intended to be used for any kind of
> security purpose, neither to fully sandbox a process nor to restrict
> filesystem system calls. In the past, chroot() has been used by daemons to
> restrict themselves prior to passing paths supplied by un‐trusted users to
> system calls such as open(2). However, if a folder is moved out of the chroot
> directory, an attacker can exploit that to get out of the chroot directory as
> well. The easiest way to do that is to chdir(2) to the to-be-moved directory,
> wait for it to be moved out, then open a path like ../../../etc/passwd.
>
> A slightly trickier variation also works under some circumstances if chdir(2)
> is not permitted. If a daemon allows a "chroot directory" to be specified,
> that usually means that if you want to prevent remote users from accessing
> files outside the chroot directory, you must ensure that folders are never
> moved out of it.
>
> This call does not change the current working directory, so that after the
> call '.' can be outside the tree rooted at '/'. In particular, the superuser
> can escape from a "chroot jail" by doing:
>
>	mkdir foo; chroot foo; cd ..
>
> This call does not close open file descriptors, and such file descriptors may
> allow access to files outside the chroot tree.

# BUGS

Report bugs at *https://github.com/gportay/iamroot/issues*

# AUTHOR

Written by Gaël PORTAY *gael.portay@gmail.com*

# COPYRIGHT

Copyright (c) 2021-2022 Gaël PORTAY

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 2.1 of the License, or (at your option) any
later version.

# SEE ALSO

[iamroot(7)], [iamroot-shell(1)], [chroot(2)], [path_resolution(7)],
[fakechroot(1)]

[alpine-make-rootfs]: https://github.com/alpinelinux/alpine-make-rootfs/blob/master/README.adoc
[chdir(2)]: https://linux.die.net/man/2/chdir
[chown(2)]: https://linux.die.net/man/2/chown
[chroot(2)]: https://linux.die.net/man/2/chroot
[fakechroot(1)]: https://linux.die.net/man/1/fakechroot
[fchdir(2)]: https://linux.die.net/man/2/fchdir
[fopen(3)]: https://linux.die.net/man/3/fopen
[iamroot(7)]: iamroot.7.adoc
[iamroot-shell(1)]: iamroot-shell.1.adoc
[open(2)]: https://linux.die.net/man/2/open
[dnf(8)]: https://dnf.readthedocs.io/en/latest/command_ref.html
[pacstrap(8)]: https://man.archlinux.org/man/pacstrap.8
[path_resolution(7)]: https://linux.die.net/man/7/path_resolution
[readlink(2)]: https://linux.die.net/man/2/readlink
[stat(2)]: https://linux.die.net/man/2/stat
