# iamroot

## TL;DR;

iamroot emulates the syscall `chroot(2)` for unprivileged processes in pure
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
jail" and in [fpath_resolutionat.c](fpath_resolutionat.c) for resolving
pathnames.

iamroot aims to provide an alternative to [fakechroot(1)], which does great but
does not run well for creating rootfs with [pacstrap(8)]. Its existing world
would likely break if it is hacked to address the rootfs-creation related
issues (i.e. fixing entering-exiting chroot and absolute symlink resolution in
short).

Of course, iamroot cannot substitute to the superuser permissions, and commands
will end with `EACCESS` or `EPERM` as of reading or writing files in `/proc`,
`/sys`, `/dev` or `/run`, to name but a few.

iamroot is a proof-of-concept, therefore expect the unexpectable!

# SEE ALSO

[chroot(2)], [path_resolution(7)], [fakechroot(1)]

[chdir(2)]: https://linux.die.net/man/2/chdir
[chown(2)]: https://linux.die.net/man/2/chown
[chroot(2)]: https://linux.die.net/man/2/chroot
[fakechroot(1)]: https://linux.die.net/man/1/fakechroot
[fchdir(2)]: https://linux.die.net/man/2/fchdir
[fopen(3)]: https://linux.die.net/man/3/fopen
[open(2)]: https://linux.die.net/man/2/open
[pacstrap(8)]: https://man.archlinux.org/man/pacstrap.8
[path_resolution(7)]: https://linux.die.net/man/7/path_resolution
[readlink(2)]: https://linux.die.net/man/2/readlink
[stat(2)]: https://linux.die.net/man/2/stat
