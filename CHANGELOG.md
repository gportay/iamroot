# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [unreleased]

### Added

- Intercept the functions `__stat64()`, `__lstat64()`, `__fstatat64()` and
  `__futimensat64()` ([glibc] only)

### Changed

- Create weak aliases for [glibc] `__` and `64` variants
- Output the parameters even in chroot for the functions `execveat()`,
  `execve()` and `posix_spawn()`
- Output a warning the if the interpretor is not handled
- Support the environment variables `argv0`, `LD_LIBRARY_PATH` and `LD_PRELOAD`
  and while running a generic ELF [dynamic loader] (i.e. neither [glibc] nor
  [musl])

### Fixed

- Fix the setting of arguments while running a generic ELF [dynamic loader]
  (i.e. neither [glibc] nor [musl])
- Fix the shifting of extra arguments while running the [dynamic loader]

## [v4] - 2022-04-25

### Added

- Support for 32-bit ELF executables
- Chroot to an i686 world; tested on [Arch Linux 32]
- Chroot to an arm world; tested on [Arch Linux ARM] and [Fedora ARM]
- Create a [Debian] and [Ubuntu] rootfs via [debootstrap(8)]
- Create a [Manjaro] rootfs via [pacstrap(8)]
- Intercept the function `__libc_start_main()` to set `argv[0]` with the
  content of the environment variable `argv0` if set
- Use the [dynamic loader][ld.so(8)] option `--inhibit-rpath` if
  [exec][exec(3)]'ing to a [glibc] chroot'ed environment
- Create an [openSUSE] ([Tumbleweed] and [Leaf]) rootfs via [zypper(8)]
- Intercept the function `execveat()`
- Add the environment variable `IAMROOT_VERSION`
- Run the [dynamic loader][ld.so(8)] without options if its path does not start
  by `/lib/ld` or `/lib64/ld`
- Use the [dynamic loader][ld.so(8)] option `--inhibit-cache` if
  [exec][exec(3)]'ing to a [glibc] chroot'ed environment
- Support for optimizations up to `-O5` and source fortification up to
  `_FORTIFY_SOURCE=2`
- Add the `iamroot-shell`'s option `--no-color`, and the environment variable
  [NO_COLOR] to colorize the debug traces; a zero value do not disable color

### Changed

- The libraries are installed by architecture in sub-directories: x86_64, i686,
  arm, armhf and aarch64
- Output the flags in octal (open flags) and in hexadecimal (AT flags)
- Output the detailled `execve` command on level 4 and above
- Ignore the `EPERM` error on functions `chmod()`, `chown()`, `fchmod()`,
  `fchmodat()`, `fchown()`, `fchownat()`, `lchmod()` and `lchown()`
- Forward the flag `O_NOFOLLOW` as flag `AT_SYMLINK_NOFOLLOW` to the function
  `path_resolution()` for the open functions `__open()`, `__open64()`,
  `__open64_2()`, `__open_2()`, `__openat64_2()`, `__openat_2()`, `open()`,
  `open64()`, `openat()` and `openat64()`
- Output the symbol name on path resolution error
- The internal function `sanitize()` do not sanitize the empty paths
- The internal function `path_resolution()` returns `ELOOP` if too many
  symbolic links were followed
- The internal function `path_resolution()` follows relative symlinks
- The internal function `path_resolution()` does not ignore `/run/systemd`
- The internal function `path_resolution()` resolves `/proc/self/exe` symlink
  with the content of the auxiliary vector `AT_EXECFN`
- The functions `path_resolution_init()`, `setegid()`, `seteuid()`, `setgid()`
  and `setuid()` check for `_snprintf()` returned value
- The internal function `path_resolution()` does not ignore the libraries
  anymore
- Output the environment pointer for the functions `execle()`, `execve()` and
  `execvpe()`,
- Output the function `execvpe()`
- Output a warning the if secure-execution mode set
- Output the fd for the functions `openat()` and `openat64()`
- Output the parameters for the functions `getcwd()` and `getwd()`
- Output all the traces in `exec.sh` whatever the debug level
- Output the exec-like parameters for the function `__posix_spawnp()`
- The internal function `path_resolution()` returns the length for the resolved
  path

### Removed

- Remove unexistant function `opendir64()`
- Remove linkage against `libdl.so` and `libpthread.so`

### Fixed

- Fix the setting for the variables `IAMROOT_LIB_<ldso>_<abi>` that override
  the path to the iamroot library to preload by [dynamic loader][ld.so(8)]; the
  variables have to be set without using the `_<abi>` suffix number: i.e. using
  `IAMROOT_LIB_<ldso>`
- Fix the setting for spurious execution bit in file mode
- Fix the forwarding of none-AT flags argument to `path_resolution()` for the
  functions `__openat64_2()`, `__openat_2()`, `openat()`, `openat64()` and
  `renameat2()`
- Fix the `argv[0]` value for the [glibc] [dynamic loader][ld.so(8)] that does
  not support the option `--argv0` before 2.33; the value in `argv[0]` is now
  stripped from the path to the chroot
- Fix the use for the [dynamic loader][ld.so(8)] option `--preload` which is
  supported since [glibc] 2.30
- Fix the adding of extra arguments due to off-by-one shifting to prepend the
  [dynamic loader][ld.so(8)] and its arguments if the command as less arguments
  that the shift
- Fix the following symlink behaviour for the at functions `readlinkat()`,
  `renameat2()`, `symlink()`, `symlinkat()` and `unlinkat()` by forwarding AT
  flag `AT_SYMLINK_NOFOLLOW` to `path_resolution()`
- Fix the adding of an empty hashbang argument
- Fix the ignoring of empty paths in the function `path_resolution()`
- Fix the handling for NULL dir argument in the function `tempnam()`
- Fix the behaviour for the functions `posix_spawn()` and `posix_spawnp()`
- Fix the handling for path argument without slash in the functions `dlopen()`
  and `dlmopen()` by loading the dynamic shared object (shared library) from
  the environment variable `IAMROOT_LD_LIBRARY_PATH`
- Fix the symbol names for `__openat64_2()` and `opendir64()`
- Fix the buffer overflow with the functions `get_current_dir_name()`,
  `getcwd()` and `getwd()`
- Fix the returning of an empty string if the buffer is allocated by the real
  symbol `getcwd()` due to an empty buffer given in parameter
- Fix the invalid reading for setting [dynamic loader][ld.so(8)] option with
  the function `execve()`

### Security

- Fix off-by-one array reading while getting `RPATH` and `RUNPATH` from ELF
  executable

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
- The internal function `fpath_resolutionat()` honors the at-flag
  `AT_EMPTY_PATH`
- The internal function `fpath_resolutionat()` ignores `/proc/self/fd/<fd>`
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
[Arch Linux 32]: https://archlinux32.org/
[Arch Linux ARM]: https://archlinuxarm.org/
[Debian]: https://www.debian.org/
[Fedora ARM]: https://arm.fedoraproject.org/
[Fedora]: https://getfedora.org/
[Leaf]: https://www.opensuse.org/#Leap
[Manjaro]: https://manjaro.org/
[NO_COLOR]: https://no-color.org/
[Tumbleweed]: https://www.opensuse.org/#Tumbleweed
[Ubuntu]: https://ubuntu.com/
[__nss_files_fopen()]: https://sourceware.org/git/?p=glibc.git;a=blob;f=nss/nss_files_fopen.c;h=594e4216578766e4534c44dc6c75283d5d1a20fe;hb=299210c1fa67e2dfb564475986fce11cd33db9ad
[alpine-make-rootfs]: https://github.com/alpinelinux/alpine-make-rootfs/blob/master/README.adoc
[busybox(1)]: https://linux.die.net/man/1/busybox
[capabilities(7)]: https://linux.die.net/man/7/capabilities
[debootstrap(8)]: https://linux.die.net/man/8/debootstrap
[dnf(8)]: https://dnf.readthedocs.io/en/latest/command_ref.html
[exec(3)]: https://linux.die.net/man/3/exec
[glibc]: https://www.gnu.org/software/libc/
[ld.so(8)]: https://linux.die.net/man/8/ld.so
[musl]: https://www.musl-libc.org/
[openSUSE]: https://www.opensuse.org/
[pacstrap(8)]: https://man.archlinux.org/man/pacstrap.8
[passwd(1)]: https://linux.die.net/man/1/passwd
[su(1)]: https://linux.die.net/man/1/su
[systemd-sysusers(8)]: https://www.freedesktop.org/software/systemd/man/systemd-sysusers.html
[unreleased]: https://github.com/gportay/iamroot/compare/v4...master
[v1]: https://github.com/gportay/iamroot/releases/tag/v1
[v2]: https://github.com/gportay/iamroot/compare/v1...v2
[v3]: https://github.com/gportay/iamroot/compare/v2...v3
[v4]: https://github.com/gportay/iamroot/compare/v3...v4
[zypper(8)]: https://en.opensuse.org/SDB:Zypper_manual_(plain)
