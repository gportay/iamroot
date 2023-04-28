# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [unreleased]

### Added

- Create weak aliases for the [glibc] functions `__openat_2()` and
  `__openat64_2()`
- Add the environment variable `IAMROOT_PATH_RESOLUTION_WORKAROUND` to disable
  the prepend of the root directory if the path to resolve contains it already
- The internal function `path_resolution()` resolves `/proc/1/exe` symlink
  to `/`
- Remove function `running_in_chroot()`
- Add the `iamroot-shell`'s option `--preserve-env` and its environment
  variable `IAMROOT_PRESERVE_ENV` to preserve the given user environment
  variables in the shell
- The internal function `path_resolution()` ignores the resolved path after the
  symlinks are followed and expanded
- Intercept the functions `accept()`, `accept4()`, `bind()`, `close()`,
  `connect()`, `dup()`, `dup2()`, `dup3()`, `fdopen()`, `fileno()`,
  `getpeername()`, `getresgid()`, `getresuid()`, `getsockname()`,
  `initgroups()`, `setregid()`, `setresgid()`, `setresuid()` and `setreuid()`

### Removed

- Remove function `mq_open()`

### Changed

- Output the directory fd for the functions `name_to_handle_at()` and
  `scandirat()`
- Use `/lib64:/usr/local/lib64:/usr/lib64` library path for both glibc
  architectures x86_64 and aarch64
- Output the full command from level 1 to 4
- Remove the relative path warning as the `proc` file-system may return also
  pipe or socket for the given fd
- Output the path of directory fd for the functions `__fxstat()`,
  `__fxstatat()`, `__fxstatat64()`, `__xmknodat()`, `chflagsat()`,
  `execveat()`, `faccessat()`, `fanotify_mark()`, `fchmodat()`, `fchownat()`,
   `fstatat()`, `fstatat64()`, `futimesat()`, `getfhat()`, `linkat()`,
   `mkdirat()`, `mkfifoat()`, `mknodat()`, `mkostempsat()`,
  `name_to_handle_at()`, `openat()`, `path_resolution()`, `readlinkat()`,
  `renameat()`, `renameat2()`, `scandirat()`, `statx()`, `symlinkat()`,
  `unlinkat()` and `utimensat()`,
- Output the fd for the functions `__fxstat()`, `__fxstat64()`, `fchmod()`,
  `fchown()`, `fexecve()`, `fstat()` and `fstat64()`
- Start the `iamroot-shell` with a clean environment (`IAMROOT_`-env, `LD_`-env
  `PATH` and `HOME`)
- The functions `__fxstat()`, `__fxstat64()`, `fstat()` and `fstat64()` do not
  call the AT-functions `__fxstatat()`, `__fxstatat64()`, `fstatat()` and
  `fstatat64()` anymore
- Output the resolved path for the function `ftok()`
- The internal function `path_resolution()` do not resolves `/proc/self/root`
  symlink with the content of the environment variable `$IAMROOT_ROOT`
- Output the ignored `errno` errors returned by the original symbols on level
  debug
- Output the errors returned by the symbols `next_extattr_delete_fd()`,
  `next_extattr_delete_link()`, `next_extattr_get_fd()`,
  `next_extattr_get_link()`, `next_extattr_set_fd()`,
  `next_extattr_set_link()`, `next_fgetxattr()`, `next_fremovexattr()`,
  `next_fsetxattr()`, `next_lgetxattr()`, `next_lremovexattr()`,
  `next_lsetxattr()` and `next_readlinkat()` on level debug

### Fixed

- Fix the output of the open flags for the function `renameat2()`
- Fix the alteration of the `errno` error in the internal function
  `__procfdreadlink()` and the functions `getegid()`, `geteuid()`, `getgid()`
  and `getuid()`
- Fix the loading of the user ownership from extended attributes of the
  resolved paths in the function `fchownat()`
- Fix the missing stripping of the chroot directory in the function
  `canonicalize_file_name()`
- Fix the truncating of symlinks read from a chroot directory if the buffer is
  too small to contain the whole symlink
- Fix the handling for empty path if dfd is not `AT_FDCWD`
- Fix the calling to the next symbol with the resolved path for the function
  `ftok()`
- Fix the handling for path argument without slash in the function `catopen()`
  by opening the catalog files from the environment variable `NLSPATH`

### Security

- Fix an overflowing if giving a fd superior to 999 (or any negative fd such as
  the special `AT_FDCWD`) to the function `__procfdname()`

## [v8] - 2022-09-20

### Added

- Save and load the user ownership to extended attributes for the functions
  `__fxstatat()`, `__fxstatat64()`, `fchownat()`, `fstatat()`, `fstatat64()`
  and `statx()`

## [v7] - 2022-09-08

### Added

- Create additional weak aliases for [glibc] `__` and `64` variants
- Intercept the [glibc] `64` variant functions `nftw64()`, `scandir64()` and
  `truncate64()`
- Intercept the functions `acct()`, `catopen()`, `ftok()`, `getegid()`,
  `getgid()`, `getgroups()`, `mq_open()` and `setgroups()`
- Intercept the Linux's functions `fanotify_mark()`, `inotify_add_watch()`,
  `swapon()` and `swapoff()`
- Create the `64` weak aliases `__fxstat64()`, `__fxstatat64()`,
  `__lxstat64()`, `__xstat64()`, `fopen64()`, `freopen64()`, `fstat64()`,
  `fstatat64()`, `lstat64()`, `open64()`, `openat64()` and `stat64()` for
  [musl] world
- Intercept the [FreeBSD]'s functions `chflags()`, `chflagsat`, `lchflags()`
  and `mkostempsat()`
- The internal function `path_resolution()` expands all symlinks
- Support tests on [FreeBSD] 13.1
- Build from an AArch64 world; tested on [FreeBSD] and [Debian]
- Output the traces for the functions `setegid()`, `seteuid()`, `setgid()` and
  `setuid()`
- Save and load the user permissions to extended attributes for the functions
  `__fxstatat()`, `__fxstatat64()`, `__xmknodat()`, `creat()`, `fchmodat()`,
  `fstatat()`, `fstatat64()`, `mkdirat()`, `mkfifoat()`, `mknodat()`, `open()`,
  `openat()` and `statx()`

### Changed

- Output the mode for the functions `euidaccess()` and `faccessat()`
- Output the three-dots `...` for the functions `futimesat()`, `lutimes()`,
  `mkstemps()` `utime()`, `utimensat()`, and `utimes()`
- Forward the none-`at` functions `access()`/`euidaccess()`, `link()`,
  `mkdir()`, `mkfifo()`, `readlink()`, `rename()`, `symlink()`, and `unlink()`
  to the `at` functions `faccessat()`, `linkat()`, `mkdirat()`, `mkfifoat()`,
  `readlinkat()`, `renameat()`, `symlinkat()`, and `unlinkat()`
- Forward the none-`at` functions `futimesat()`, `lutimes()`, `utime()`, and
  `utimes()` to the `at` function `utimensat()`
- The function `eaccess()` is now a weak alias to `euidaccess()`
- Output traces in `exec.sh` starting from debug level 2
- Forward the functions `mkstemp()` and `mkstemps()` to the function
  `mkostemps()`
- Update the [FreeBSD]'s x86 64-bit architecture name to `amd64`
- Update the library path order to `/lib:/usr/local/lib:/usr/lib`
- Output the traces for the functions `getegid()`, `geteuid()`, `getgid()` and
  `getuid()` even if their according environment variables `IAMROOT_EGID`,
  `IAMROOT_EUID`, `IAMROOT_GID` and `IAMROOT_UID` are unset

### Removed

- Remove [glibc] private function `__nss_files_fopen()`
- Remove internal functions `whereami()` and `whoami()`
- Remove [glibc] specific function `tmpnam_r()` from none-glibc build
- Remove functions `__openat_2()` and `__opendirat()`

### Fixed

- Fix the invalid argument returned if an empty path is resolved but the flag
  `AT_EMPTY_PATH` remains for the `at` functions `__fxstatat64()`,
  `fstatat64()` and `statx()`,
- Fix the handling for path argument without slash in the functions `dlopen()`
  and `dlmopen()` by loading the dynamic shared object (shared library) from
  the environment variable `IAMROOT_LD_LIBRARY_PATH`

## [v6] - 2022-06-30

### Added

- Compile for [FreeBSD] 13.1
- Intercept the [FreeBSD]'s functions `__opendir2()`, `exect`, `execvP()`
  `extattr_delete_fd()`, `extattr_delete_file()`, `extattr_delete_link()`,
  `extattr_get_fd()`, `extattr_get_file()`, `extattr_get_link()`,
  `extattr_list_fd()`, `extattr_list_file()`, `extattr_list_link()`,
  `extattr_set_fd()`, `extattr_set_file()`, `extattr_set_link()`, `mount()`,
  `nmount()`, `scandir_b()`, and `unmount()`
- Intercept the functions `flistxattr()`, `fremovexattr()`, `ftw()` and
  `nftw()`
- Use `kinfo_getfile()` to obtain the fd's path on [FreeBSD]
- Preload the `NEEDED` libraries using `--preload` or `LD_PRELOAD`

### Changed

- Output the updated extended attribute name for the functions `fgetxattr()`,
  `fsetxattr()`, `getxattr()`, `lgetxattr()`, `lremovexattr()`, `lsetxattr()`,
  `removexattr()` and `setxattr()`
- Remove the suffix `_<ldso>_<abi>` in the variables `ld_library_path` and
  `ld_preload`
- Forward the `AT` flag `AT_EMPTY_PATH` from the functions `fchmod()` and
  `fchown()` to `fchmodat()` and `fchownat()`; except for `chmod()` on Linux 
- Be kind and do not prepend the root directory twice if it is part of the path
  to resolve already

### Fixed

- Fix the returned value for the functions `listxattr()` and `llistxattr()` if
  empty extended attribute names are encountered (i.e. "user.iamroot.")
- Fix the out-of-bound calculation of extended attribute name for the functions
  `listxattr()` and `llistxattr()`
- Fix the checking for the none-user extended attributes namespaces by checking
  against the full prefix `user.iamroot.`
- Fix the string copy up to the buffer size - 1 bytes for the internal macro
  `_strncpy()`; the macro copied at most buffer size - 2 bytes
- Fix the invalid reading for setting [dynamic loader][ld.so(8)] option with
  the function `execve()` once again
- Fix the segmentation fault if `LD_PRELOAD` is unset for the linux ELF dynamic
  loaders
- Fix the setting of the `LD_PRELOAD` for the generic ELF dynamic loader

## [v5] - 2022-06-24

### Added

- Intercept the functions `__stat64()`, `__lstat64()`, `__fstatat64()` and
  `__futimensat64()` ([glibc] only)
- Intercept the command [mountpoint(1)] in `exec.sh`
- Add the `iamroot-shell`'s option `--debug-allow` and its environment variable
  `IAMROOT_DEBUG_ALLOW` to allow output the debug traces for the given
  functions
- Add the `iamroot-shell`'s option `--path-resolution-allow` and its
  environment variable `IAMROOT_PATH_RESOLUTION_ALLOW` to resolve the given
  paths in the chroot, even if the path is ignored (i.e. the allow regex takes
  precedence the ignore regex)
- The internal function `path_resolution()` handles the at-flag `AT_EMPTY_PATH`
- Intercept the function `dl_iterate_phdr()`
- Add the `iamroot-shell`'s options `--profile-file` and `--rc-file` to
  customize startup
- Handle the environment variables `IAMROOT_LD_PRELOAD_LINUX_3`,
  `IAMROOT_LD_PRELOAD_LINUX_ARMHF_3`, `IAMROOT_LD_PRELOAD_LINUX_AARCH64_1` and
  `IAMROOT_LIB_LINUX_3` to override the list of [glibc] armhf libraries to be
  preloaded

### Changed

- Create weak aliases for [glibc] `__` and `64` variants
- Output the parameters even in chroot for the functions `execveat()`,
  `execve()` and `posix_spawn()`
- Output a warning if the interpretor is not handled
- Support the environment variables `argv0`, `LD_LIBRARY_PATH` and `LD_PRELOAD`
  while running a generic ELF dynamic loader
- Remove the library path prefix `LD_` from the environment variable
  `IAMROOT_LD_LIBRARY_PATH` and the prefix `-ld` from the script option
  `--ld-library-path`
- The functions `__fxstat()`, `__fxstat64()`, `__lxstat()`, `__lxstat64()`,
  `__xstat()`, `__xstat64()`, `fstat()`, `fstat64()`, `lstat()`, `lstat64()`,
  `stat()` and `stat64()` calls the functions `__fxstatat()`, `__fxstatat64()`,
  `fstatat()` and `fstatat64()`
- The functions `fchmod()` and `fchown()` calls the functions `fchownat()` and
  `fchownat()`
- Output the architecture, the libc and the pid on level 6 and above
- Output the architecture and the libc as platform
- Output the `EPERM` and `EACCES` `errno` errors returned by the original
  symbols on level 2
- Output the `root` directory for the functions `__fpathperror()`,
  `__pathperror()` and `__pathperror2()`
- Output the `ENOSYS` `errno` error returned by the original symbols on the
  notice debug level

### Deprecated

- Replace the `iamroot(7)` environment variable `IAMROOT_LD_LIBRARY_PATH` by
  the environment variable `IAMROOT_LIBRARY_PATH`
- Replace the `iamroot-shell(1)` option `--ld-library-path` by the option
  `--library-path`

### Fixed

- Fix the setting of arguments while running a generic ELF dynamic loader
- Fix the shifting of extra arguments while running the dynamic loader
- Fix the invalid argument returned if an empty path is resolved but the flag
  `AT_EMPTY_PATH` remains for the `at` functions `__fxstatat()`, `execveat()`,
  `faccessat()`, `fchmodat()`, `fchownat()`, `fstatat()`, `linkat()`,
  `name_to_handle_at()`, `unlinkat()` and `utimensat()`
- Fix the execution of un-handled commands from the host environment while
  running the script `exec.sh`
- Fix the value of `IAMROOT_LIB` accross the `execve` calls

### Security

- Fix an overflowing while using `gpg` by adding extra padding after `regex_t`
  structures (starting from gpg 2.2.22) once again

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
- Create an [openSUSE] ([Tumbleweed] and [Leap]) rootfs via [zypper(8)]
- Intercept the function `execveat()`
- Add the environment variable `IAMROOT_VERSION`
- Run the [dynamic loader][ld.so(8)] without options if its path does not start
  by `/lib/ld` or `/lib64/ld`
- Use the [dynamic loader][ld.so(8)] option `--inhibit-cache` if
  [exec][exec(3)]'ing to a [glibc] chroot'ed environment
- Support for optimizations up to `-O5` and source fortification up to
  `_FORTIFY_SOURCE=2`
- Add the `iamroot-shell`'s option `--no-color` and the environment variable
  [NO_COLOR] to colorize the debug traces; a zero value do not disable color

### Changed

- The libraries are installed by architecture in sub-directories: x86_64, i686,
  arm, armhf and aarch64
- Output the flags in octal (open flags) and in hexadecimal (`AT` flags)
- Output the detailled `execve` command on level 4 and above
- Ignore the `EPERM` error on functions `chmod()`, `chown()`, `fchmod()`,
  `fchmodat()`, `fchown()`, `fchownat()`, `lchmod()` and `lchown()`
- Forward the flag `O_NOFOLLOW` as flag `AT_SYMLINK_NOFOLLOW` to the function
  `path_resolution()` for the open functions `__open()`, `__open64()`,
  `__open64_2()`, `__open_2()`, `__openat64_2()`, `__openat_2()`, `open()`,
  `open64()`, `openat()` and `openat64()`
- Output the symbol name on path resolution error
- The internal function `sanitize()` does not sanitize the empty paths
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
- Fix the forwarding of none-`AT` flags argument to `path_resolution()` for the
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
- Fix the following symlink behaviour for the `at` functions `readlinkat()`,
  `renameat2()`, `symlink()`, `symlinkat()` and `unlinkat()` by forwarding `AT`
  flag `AT_SYMLINK_NOFOLLOW` to `path_resolution()`
- Fix the adding of an empty hashbang argument
- Fix the ignoring of empty paths in the function `path_resolution()`
- Fix the handling for NULL dir argument in the function `tempnam()`
- Fix the behaviour for the functions `posix_spawn()` and `posix_spawnp()`
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
  `setegid()`, `seteuid()`, `setgid()` and `setuid()`
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
  `putgrent()` and `getgrouplist()` that are stolen from [musl]
- Intercept the passwd functions `getpwnam_r()`, `getpwuid_r()`, `fgetpwent()`,
  `getpwuid()`, `setpwent()`, `endpwent()`, `getpwuid()`, `getpwnam()` and
  `putpwent()` that are stolen from [musl]
- Intercept the shadow functions `getspnam_r()`, `getspnam()`, `fgetspent()`,
  `getspent()`, `setspent()`, `endspent()`, `putspent()`, `lckpwdf()` and
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
  options `--preload` and `--library-path`
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
- Split the none-related build targets in `Makefile` to the new separate file
  `support/makefile`
- The script `exec.sh` is now a `/bin/sh` script
- The script `exec.sh` runs un-handled commands using the executables from the
  host environment
- The internal function `fpath_resolutionat()` honors the at-flag
  `AT_EMPTY_PATH`
- The internal function `fpath_resolutionat()` ignores `/proc/self/fd/<fd>`
- Set the at-flag `AT_SYMLINK_NOFOLLOW` at path resolution for the change
  timestamp functions `futimesat()`, `utime()`, `utimensat()` and `utimes()`
- Uses `IAMROOT_[GU]ID` to specify the "real" user/group id and
  `IARMTOO_E[GU]ID` to set specify the "effective" user/group id

### Deprecated

- Replace the `iamroot(7)` environment variable `IAMROOT_GETEUID` by the
  environment variable `IAMROOT_EUID`

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
[FreeBSD]: https://www.freebsd.org/
[Leap]: https://www.opensuse.org/#Leap
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
[mountpoint(1)]: https://linux.die.net/man/1/mountpoint
[musl]: https://www.musl-libc.org/
[openSUSE]: https://www.opensuse.org/
[pacstrap(8)]: https://man.archlinux.org/man/pacstrap.8
[passwd(1)]: https://linux.die.net/man/1/passwd
[su(1)]: https://linux.die.net/man/1/su
[systemd-sysusers(8)]: https://www.freedesktop.org/software/systemd/man/systemd-sysusers.html
[unreleased]: https://github.com/gportay/iamroot/compare/v8...master
[v1]: https://github.com/gportay/iamroot/releases/tag/v1
[v2]: https://github.com/gportay/iamroot/compare/v1...v2
[v3]: https://github.com/gportay/iamroot/compare/v2...v3
[v4]: https://github.com/gportay/iamroot/compare/v3...v4
[v5]: https://github.com/gportay/iamroot/compare/v5...v6
[v6]: https://github.com/gportay/iamroot/compare/v6...v5
[v7]: https://github.com/gportay/iamroot/compare/v7...v8
[v8]: https://github.com/gportay/iamroot/compare/v8...master
[zypper(8)]: https://en.opensuse.org/SDB:Zypper_manual_(plain)
