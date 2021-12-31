# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [unreleased]

## [v3] - 2021-12-31

### Added

- Chroot to an AArch64 world; tested on [Arch Linux ARM], [Fedora ARM] and
  [Alpine Linux]
- Create a [Fedora] rootfs via [dnf(8)]
- Intercept the functions `umask()`, `fchmod()`, `fgetxattr()`, `fsetxattr()`,
  `getegid()`, `getgid()`, `setegid()`, `seteuid()`, `setgid()` and `setuid()`
- Update and add a warning if the mode is lacking for user permissions in the
  functions `__open()`, `__open64()`, `__xmknod()`, `__xmknodat()`, `chmod()`,
  `creat()`, `creat64()`, `fchmodat()`, `lchmod()`, `mkdir()`, `mkdirat()`,
  `mkfifo()`, `mkfifoat()`, `mknod()`, `mknodat()`, `open()`, `open64()`,
  `openat()` and `openat64()`
- Add the environment variables `IAMROOT_EGID`, `IAMROOT_EUID`, `IAMROOT_GID`
  and `IAMROOT_UID` to specify a custom id for either the "real" or the
  "effective" group or user
- Intercept the group functions `getgrnam_r()`, `getgrgid_r()`, `fgetgrent()`,
  `getgrent()`, `setgrent()`, `endgrent()`, `getgrgid()`, `getgrnam()`,
  `putgrent()`, and `getgrouplist()` that are stolen from [musl]
- Intercept the passwd functions `getpwnam_r()`, `getpwuid_r()`, `fgetpwent()`,
  `getpwuid()`, `setpwent()`, `endpwent()`, `getpwuid()`, `getpwnam()`, and
  `putpwent()` that are stolen from [musl]
- Intercept the shadow functions `getspnam_r()`, `getspnam()`, `fgetspent()`,
  `getspent()`, `setspent()`, `endspent()`, `putspent()`, `lckpwdf()`, and
  `ulckpwdf()` that are stolen from [musl]
- Link to the `phread` library because of using the [musl] implementation of
  group, passwd and shadow functions
- Run the [dynamic loader][ld.so(8)] `/lib64/ld*` and `/usr/bin/ld.so` (from
  [glibc] 2.34) directly
- Run the [dynamic loader][ld.so(8)] from the chroot'ed environment
- Use the [dynamic loader][ld.so(8)] option `--argv0` (if supported) if
  [exec][exec(3)]'ing to a [glibc] or [musl] chroot'ed environment
- Add the environment variables `IAMROOT_LD_PRELOAD_<ldso>_<abi>` and
  `IAMROOT_LIB_<ldso>_<abi>` to override the list of libraries to be preloaded
  by the [dynamic loader][ld.so(8)] in the chroot'd environment; the two
  libraries `/usr/lib64/libc.so.6` and `/usr/lib64/libdl.so.2` are preloaded in
  a [glibc]'s chroot'ed environment
- Add the two [exec][exec(3)] environment variables `IAMROOT_EXEC_LD_ARGV1` and
  `IAMROOT_EXEC_HASHBANG_ARGV1` to add an extra argument to debug the
  [execve][exec(3)]'ed command
- Add the `iamroot-shell`'s option `--fatal` and its environment variable
  `IAMROOT_FATAL` to raise `SIGABRT` on symbols that return either `EPERM` or
  `EACCES`
- Add the `iamroot-shell`'s option `--debug-ignore` and its environment
  variable `IAMROOT_DEBUG_IGNORE` to ignore output the debug traces for the
  given functions
- Add the `iamroot-shell`'s option `--debug-fd` and its environment variable
  `IAMROOT_DEBUG_FD` to output the debug traces to the given fd; it duplicates
  the `stderr` fd if the given fd does not exist
- Add the two `iamroot-shell`'s options `--path-resolution-ignore` and
  `--exec-ignore` to set their corresponding existing environment variables
  `IAMROOT_PATH_RESOLUTION_IGNORE` and `IAMROOT_EXEC_IGNORE` to ignore
  `path_resolution()` and `execve()` for the given paths
- Auto-complete the `iamroot-shell`'s options `--path` and `--ld-library-path`
- Add the internal header `iamroot.h` to define various functions and macros
  used by several sources accross the project
- Add the macro `NVERBOSE` at compiled time to make library quiet

### Changed

- Replace the use of `LD_PRELOAD` and `LD_LIBRARY_PATH` by the [ld.so(8)]'s
  options `--preload` and `--ld-library-path`
- Prefix the none-user extended attributes namespaces by `user.iamroot` that
  requires root privileges
- Rework the debug outputs using new introduced levels `warning`, `notice` and
  `info`
- Replace the internal function `__fprintf()` by both functions `__dprintf()`
  and `__verbosef()`
- Output the mode in octal
- Output the extended attribute name
- Output the fd's procname for the fd's functions `__fstat()`, `__fstat64()`,
    `__fxstat()`, `__fxstat64()`, `fchdir()`, `fchmod()`, `fchown()`,
  `fgetxattr()`, `fsetxattr()`, `fstat()` and `fstat64()`
- Output the `dl` and `errno` and `dl` error (if not `EPERM or `EACCES`)
  returned by the original symbols on the notice debug level
- Specify the regular expression compilation flag `REG_NOSUB` to not report the
  position of matches as they are not used
- Split the target `install` to targets to the mutliple specific targets
  `install-exec`, `install-doc` and `install-bash-completions`
- Split the non-related build targets in `Makefile` to the new separate file
  `support/makefile`
- The script `exec.sh` is now a `/bin/sh` script
- Ths script `exec.sh` runs un-handled commands using the executables from the
  host environment
- The internal function sanitize honors the at-flag `AT_EMPTY_PATH`
- The internal function sanitize ignores `/proc/self/fd/<fd>`
- Set the at-flag `AT_SYMLINK_NOFOLLOW` at path resolution for the change
  timestamp functions `futimesat()`, `utime()`, `utimensat()` and `utimes()`
- Uses `IAMROOT_[GU]ID` to specify the "real" user/group id and
  `IARMTOO_E[GU]ID` to set specify the "effective" user/group id

### Deprecated

- Replace the variable environment `IAMROOT_GETEUID` by `IAMROOT_EUID`

### Fixed

- Fix the prototypes for the three functions `__xmknod()`, `__xmknodat()` and
  `fxstatat64()` that mismatch the prototype of their original symbol
- Fix the calling to the original symbols `fstat()`, `fstat64()`, `fstatat()`,
  `fstatat64()`, `lstat()`, `lstat64()`, `stat()` and `stat64()` by falling
  back to the symbols `__xstat()` or `__fxstatat` if the original symbol is
  hidden (prior to [glibc] 2.33)
- Fix the calling to the original symbols `__xmknod()`, `__xmknodat()`,
  `dlopen()` and `dlmopen()` by using the real path that is resolved by the
  function `path_resolution()`
- Fix a building issue by updating the prototype of the two functions
  `tmpnam()` and `tmpnam_r()` that have changed since [glibc] 2.34
- Fix the path sanitization for relative paths starting by `.//...` that were
  transformed to the absolute path `/...`
- Fix running `ldconfig` from within `exec.sh` by making sure the files in
  `/etc/ld.so.conf` uses aboslute paths

### Security

- Fix an overflowing while using `snprintf()` by checking the returned value
- Fix an overflowing while using `gpg` by adding extra padding after `regex_t`
  structures (starting from gpg 2.2.22)

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
[Arch Linux ARM]: https://archlinuxarm.org/
[Fedora ARM]: https://arm.fedoraproject.org/
[Fedora]: https://getfedora.org/
[__nss_files_fopen()]: https://sourceware.org/git/?p=glibc.git;a=blob;f=nss/nss_files_fopen.c;h=594e4216578766e4534c44dc6c75283d5d1a20fe;hb=299210c1fa67e2dfb564475986fce11cd33db9ad
[alpine-make-rootfs]: https://github.com/alpinelinux/alpine-make-rootfs/blob/master/README.adoc
[busybox(1)]: https://linux.die.net/man/1/busybox
[capabilities(7)]: https://linux.die.net/man/7/capabilities
[dnf(8)]: https://dnf.readthedocs.io/en/latest/command_ref.html
[exec(3)]: https://linux.die.net/man/3/exec
[glibc]: https://www.gnu.org/software/libc/
[ld.so(8)]: https://linux.die.net/man/8/ld.so
[musl]: https://www.musl-libc.org/
[passwd(1)]: https://linux.die.net/man/1/passwd
[su(1)]: https://linux.die.net/man/1/su
[systemd-sysusers(8)]: https://www.freedesktop.org/software/systemd/man/systemd-sysusers.html
[unreleased]: https://github.com/gportay/iamroot/compare/v3...master
[v1]: https://github.com/gportay/iamroot/releases/tag/v1
[v2]: https://github.com/gportay/iamroot/compare/v1...v2
[v3]: https://github.com/gportay/iamroot/compare/v2...v3
