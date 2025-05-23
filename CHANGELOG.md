# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [unreleased]

### Added

- Intercept the functions `dladdr()` and `dladdr1()`
- Introduce internal function `path_resolution2()` with private flags;
  `PATH_RESOLUTION_NOMAGICLINKS`: disallow all magic-link resolution during
  path resolution: i.e. `/proc/1/root` for now, it is readable by root only, it
  is often use to check if the process is in chroot
  `PATH_RESOLUTION_NOIGNORE`: disallow non-resolution of ignored path during
  path resolution: i.e. all path are resolved even if it matches the regex set
  in `PATH_RESOLUTION_IGNORE`
  `PATH_RESOLUTION_NOWALKALONG`: disallow the final walkalong resolution of the
  given path
- Chroot to a [Debian] ppc world; tested on [Debian PowerPC Port]
- Add the `ish(1)`'s option `--login` to read and execute commands from the
  files `$HOME/.ish_profile` and `$PWD/.ishrc` if these files exist and prefix
  the `argv0` by the hyphen character `-`
- `ido(1)` sets the `ish(1)` option `--login` if login shell

## Removed

- Remove the function `dlopen()` from NetBSD build

### Changed

- Use [Jim Tcl]
- Do not reset the environment variable `IAMROOT_LIB` to the iamroot library
- `ish(1)` is now using `ld-iamroot.so(8)`
- Do not forward the none-`at` functions `access()` and `euidaccess()` to the
  `at` functions `faccessat()`
- The functions `__lstat50()`, `__lstat64_time64()`, `lstat()` and `lstat64()`
  do not call the AT-functions `__fstatat64_time64()`, `fstatat()` and
  `fstatat64()` anymore
- Output the traces for the fts, group, passwd, and shadow functions
- `ido(1)` sets the login shell option `-l` if `bash(1)` only for a login shell
- `ido(1)` sets the login `bash(1)` long option `--login` instead of short
  option `-l` for a login shell
- `ido(1)` sets the login `bash(1)` long option `--login` if an option command
  is set only for a login shell
- Output the returned value for the function `acct()`, `__mknodat()`,
  `getgroups()` and `mknodat()`

### Fixed

- Fix the `ido(1)`'s PATH and absolute path to `ish(1)` for [NetBSD]
- Fix the abnormal termination condition on Linux due to an invalid pointer in
  memory reallocation in function `fts_sort()`

## [v23] - 2024-06-25

### Added

- Chroot to an [Adélie Linux] x86_64 world
- Chroot to an [Adélie Linux] i686 world
- Chroot to an [Adélie Linux] aarch64 world
- Chroot to an [Adélie Linux] armv7 world
- Chroot to an [Adélie Linux] ppc64 world
- Chroot to an [Adélie Linux] ppc world
- Chroot to a PowerPC glibc world; tested on [Arch Linux Power]

### Changed

- Output the filesystem type for the function `mount()`
- Output the traces for the functions `clearenv()`, `getenv()`, `putenv()`,
  `setenv()` and `unsetenv()`

### Fixed

- Fix the `ld-iamroot.so(8)` option `--cwd`
- Fix the alteration of the `errno` error in the functions `__open_2()`,
  `__openat_2()`, `close()`, `dup()`, `dup2()`, `dup3()`, `fopen()`,
  `freopen()`, `open()` and `openat()`,
- Fix the output of the fd path the functions `close()`, `dup()`, `dup2()` and
  `dup3()`
- Fix the segmentation fault if the `ld-iamroot.so` binary has no `DT_RUNPATH`
  attribute set
- Fix the detection of the PowerPC64{,le} [glibc]'s dynamic loader
- Fix the corrupted setting of the environnement variable `IAMROOT_LIB` if the
  value comes from the environnement
- Fix the loss for environment if exec'ing from a shell using a local copy for
  envp instead of environ(7)

## [v22] - 2024-06-04

### Added

- Look up the iamroot libraries and `exec.sh` in the directory specified in the
  environment variable `IAMROOT_ORIGIN` if set first and in the iamroot library
  directory then
- Set the environment variable `IAMROOT_ORIGIN` if it is unset, using either
  the `DT_RUNPATH` value set in `ld-iamroot.so(8)` or the hard-coded iamroot
  library directory
- Set the `DT_RUNPATH` of `ld-iamroot.so(8)` to the iamroot library directory
- Add the `ld-iamroot.so(8)` option `--origin` to set the iamroot library
  directory
- Create the `64` weak aliases `__futimesat64()`, `__lutimes64()`,
  `__utime64()` and `__utimes64()`
- Run empty executable files via the bourne shell interpreter
- Create a [Mobian] rootfs via [mmdebstrap(1)]
- Chroot to a [Mobian] arm64 world
- Chroot to a [Mobian] armhf world

### Changed

- `ido(1)` is now using `ish(1)`

### Deprecated

- Replace the environment variables `IAMROOT_LIB` by the environment variable
  `IAMROOT_ORIGIN`

### Fixed

- Fix the segmentation fault if the iamroot library is preloaded more than once
  because of different locations

## [v21] - 2024-04-09

### Added

- Dectect the Aarch64 [NetBSD]'s library
- Add the `ld-iamroot.so(8)` option `--multiarch` to use [Debian multiarch]
  library path in chroot
- Set the environment variable `PWD` to `/` in `ido(1)` and `ish(1)` if
  chroot'ed
- Intercept the [NetBSD]'s functions `getgroupmembership()`, `__fstat50()`,
  `__lstat50()` and `__stat50()`
- Return `ERANGE` if the shared object is zero length
- Steal the regex functions `regcomp()`, `regexec()`, `regerror()` and
  `regfree()` from [Jim Tcl]

### Removed

- Remove the detailled `execve` output command on level 4 and above
- Remove the functions `catopen()`, `fopen()`, `freopen()`, `mkdtemp()`,
  `mkostemp()`, `mkostemps()`, `mkstemp()`, `mkstemps()`, `mktemp()`,
  `opendir()` and `tempnam()` on [NetBSD]
- Remove the output of the ignored `errno` errors returned by the original
  symbols for the functions `chown()`, `fchown()`, `fchownat()`, `lchown()`,
  `chown()`, `fchown()`, `fchownat()` and `lchown()`
- Remove the `ido`'s and `ish`'s option `--fatal` and its environment variable
  `IAMROOT_FATAL`

### Changed

- Double quote the arguments of full command if the argument contains a
  whitespace
- Search for the executable in `PATH` in `ld-iamroot.so(8)` if the program if a
  filename without a slash
- Do not output for the dynamic loader command in `exec.sh` to not pollute its
  output on stderr
- Run `exec.sh` if program a statically linked ELF executable even if not
  chroot'ed
- Output a warning if a using the default library
- Output a notice if exec'ing a SUID or static-pie executable
- Preload the iamroot library not using `IAMROOT_LIB` if `LD_PRELOAD` is unset
  and not in a chroot'ed environment
- Strip leading `ld_library_path` from the content of `LD_LIBRARY_PATH`

### Fixed

- Fix the if statement if `_snprintf()` returns -1 and `ENAMETOOLONG`
- Fix the behaviour for the function `ttyname_r()`
- Fix the behaviour for the function `getgrouplist()` on \*BSD systems
- Fix the testing for regex functions `regcomp()` and `regexec()` returned
  values
- Fix the regex subexpression emptyness for library and dynamic loader on \*BSD
  systems

## [v20] - 2024-03-08

### Added

- Preload the iamroot library if `LD_PRELOAD` is unset and not in a chroot'ed
  environment
- Add `ld-iamroot.so`, the [ld.so(8)] CLI for `iamroot(7)`

### Removed

- Remove the environment variables `ld_preload`, `ld_library_path` and
  `inhibit_rpath`

### Fixed

- Fix the out-of root directory creation of temporary files or directories for
  the functions `mktemp()` and `mkdtemp()`
- Fix the segmentation fault if preloading a shared-object with a circular
  dependency
- Fix the detection of the [FreeBSD], [NetBSD] and [OpenBSD]'s libraries

## [v19] - 2024-03-05

### Added

- Chroot to a musl s390x world; tested on [Alpine Linux]
- Chroot to a musl ppc64le world; tested on [Alpine Linux]
- Output a warning if a library is not found in the library-path
- Output a warning if the iamroot library is not found in the library-directory
- Output a warning if the iamroot library is not preloaded

### Changed

- Output traces even if not in chroot
- Output the returned value for the function `extattr_set_link()`

### Fixed

- Fix the detection of the PowerPC64{,le} [musl]'s library
- Fix the invalid argument version value given to `__fxstat{,at}{,64}()` on
  `mips` and `i386`

## [v18] - 2024-02-14

### Added

- Add the `ido(1)` and `ish(1)` option `--multiarch` and its boolean
  environment variable `IAMROOT_MULTIARCH` to use [Debian multiarch] library
  path in chroot
- Chroot to a PowerPC64 glibc world; tested on [Arch Linux Power]
- Chroot to a [Debian] mips64el world; tested on [Debian MIPS Port]
- Add the `ido(1)` and `ish(1)` option `--libdir` set `/lib:/usr/lib` as
  default library path in chroot

### Removed

- Remove the `ish(1)`'s environment variables `ISH_COMMAND`, `ISH_GID`,
  `ISH_UID` and `ISH_USER`
- Remove the `ish(1)`'s option `--user` and its environment variable
  `IAMROOT_USER` to usurp the user identity
- Remove the `ish(1)`'s option `--preserve-env` and its environment variable
  `ISH_PRESERVE_ENV` to preserve the given user environment variables in the
  shell

### Changed

- The environment variable `IAMROOT_GROUPS` is now using whitespace delimiter
  as the `bash` array variable `GROUPS`
- The internal function `path_resolution()` returns `ENAMETOOLONG` if the given
  path exceeds `PATH_MAX` - 1
- The functions `execl()`, `execle()`, `execlp()`, `execve()`, `execveat()` and
  `posix_spawn()` return `E2BIG` if the argument list is too long
- Do not restrict the environment variable `IAMROOT_PATH_RESOLUTION_AF_UNIX` to
  a chroot environment if the Unix socket path exceeds the limitation of 108
  characters

### Fixed

- Fix the behaviour of the `ido`'s options `--user` and `--group` that does set
  the real and effective ID
- Fix the detection of the [NetBSD]'s [dynamic loader][ld.so(8)]
- Fix the missing copy of the NULL character if sanitizing a path starting by
  `./`
- Fix the resolution path of an Unix socket path exceeding the 108 characters

### Security

- Fix stack buffer overflow if sanitizing a path starting by `./`

## [v17] - 2024-01-30

### Added

- Preserve the environment variable `NO_COLOR` in `ish`
- Add `ido`, the `sudo(8)` CLI for `iamroot(7)`
- Chroot to a musl armhf world; tested on [Alpine Linux]
- Fixup the content of `LD_LIBRARY_PATH` used by `ld.so(8)`, `ldd(1)` and
  `dlopen()` for \*BSD and unrecognized [dynamic loaders][ld.so(8)]
- Chroot to a [Debian] mipsel world; tested on [Debian MIPS Port]
- Support different endianness; tested on s390x from x86_64
- Chroot to a [Debian] s390x world; tested on [Debian S/390 Port]
- Chroot to an AArch64 BE; untested
- Chroot to a [Debian] ppc64el world; tested on [Debian PowerPC64 LE Port]
- Chroot to a musl ppc64le world; tested on [Alpine Linux]
- Implement the environment function `clearenv()`, `getenv()`, `putenv()`,
  `setenv()` and `unsetenv()` for internal purpose
- Chroot to a PowerPC64 LE world; tested on [Arch Linux Power]

### Changed

- Change the working directory to `IAMROOT_ROOT` in `ish` if the option
  `--root` is set before running shell
- Set or prefix the path directories in variable `LD_LIBRARY_PATH` by the
  content of the variable `IAMROOT_ROOT` variable in `ish` before the running
  the shell
- Add the environment variables `IAMROOT_LIB_<ldso>`, `IAMROOT_LIB_<abi>`,
  `IAMROOT_DEFLIB_<ldso>` and `IAMROOT_DEFLIB_<abi>` if the
  [dynamic loader][ld.so(8)] ldso or abi are not part of the filename.
- Replace the environment variables `IAMROOT_LIB` and `IAMROOT_DEFLIB` by
  `IAMROOT_LIB_<machine>` and `IAMROOT_DEFLIB_<machine>`
- Keep environment variables `ld_preload` and `ld_library_path` if \*BSD and
  unrecognized [dynamic loaders][ld.so(8)]

### Removed

- Remove the `ish`'s environment variable `ISH`

### Fixed

- Fix the behaviour of the `ish`'s option `--shell` that does not sets the
  shell interpreter
- Fix the behaviour of the `ish`'s option `--debug-fd` that does not duplicate
  stderr to the given fd if it exists and always duplicates stderr to new fd
  instead
- Fix the setting of the `ish`'s environment variable `ISH_USER` to mismatch
  `USER` iamroot's value instead of `USER` user's value
- Fix the detection of the [NetBSD]'s [dynamic loader][ld.so(8)]
- Fix the regression on rewriting the command-line with interpreter scripts on
  non-[glibc] and non-[musl] worlds since [v13] (i.e. on \*BSD systems)
- Fix the no-doing environment functions if running GNU Bash

## [v16] - 2024-01-04

### Added

- Preserve the environment variable `TERM` in `ish`

### Removed

- Remove the `ish`'s option `--path` and its environment variable
  `IAMROOT_PATH`

### Fixed

- Fix the soname lookup from the cache file `/etc/ld.so.cache`

## [v15] - 2023-12-09

### Fixed

- Fix the memory area overlapping if the root directory is stripped off once
  again
- Fix the detection of the path to the library to preload
- Fix the mis-completion of the option `--deflib`

## [v14] - 2023-11-15

### Added

- Chroot to a [Debian] armel world; tested on [Debian ARM EABI Port]
- Chroot to a [Debian] armhf world; tested on [Debian ARM hard-float Port]
- Chroot to a [Debian] arm64 world; tested on [Debian ARM 64 Port]
- Chroot to a [Debian] riscv64 world; tested on [Debian RISC-V 64 Port]
- Chroot to a [Debian] i386 world; tested on [Debian i386 Port]
- Chroot to a [glibc] pre-[ld.so(8)]'s `--preload` world; tested on [Fedora
  30], based on [glibc 2.29]

### Removed

- Remove the preload of `libdl.so` and `libpthread.so` in `ish`

### Fixed

- Fix the interception of the `_time64` variant functions `__fstat64_time64()`,
  `__fstatat64_time64()`, `__lstat64_time64()` and `__stat64_time64()` on
  32-bit architectures ([glibc] on x86, ARM and ARM hard-float)
- Fix the segmentation fault if the [dynamic loader][ld.so(8)] is run with no
  argument from within a chroot environment
- Fix the no-doing environment functions in [glibc] world by setting a bare
  minimal environment instead of `__environ` to run the script `exec.sh`

## [v13] - 2023-10-11

### Added

- Add the environment variable `IAMROOT_PATH_RESOLUTION_AF_UNIX` to reduce the
  path resolution of an in-chroot Unix socket if it exceeds the limitation of
  108 characters
- Chroot to a musl arm world; untested

### Changed

- Link dynamically against libc on [musl] and on \*BSD systems ([FreeBSD],
  [OpenBSD] and [NetBSD])
- Add the `DT_NEEDED` [glibc] shared object manually via `patchelf` in order to
  continue to not link libc at link-time and leak [glibc] symbols
- Replace the `exec.sh`'s environment variables `argv0`, `ld_preload` and
  `ld_library_path` by `_argv0`, `_preload` and `_library_path`
- Restrict the environment variable `IAMROOT_VERSION` to the script `exec.sh`
- The internal function `path_resolution()` returns `ENAMETOOLONG` if the given
  buffer is too small

### Fixed

- Fix the copying back of the generated temporary filename to the template for
  the functions `mkdtemp()`, `mkostemp()`, `mkostempsat()`, `mkostemps()`,
  `mkstemp()`, `mkstemps()` and `mktemp()`

## [v12] - 2023-09-15

### Added

- Intercept the functions `__getcwd_chk()`, `__getgroups_chk()`,
  `__getwd_chk()`, `__ptsname_r_chk()`, `__readlink_chk()`,
  `__readlinkat_chk()`, `__realpath_chk()`, `__ttyname_r_chk()`,
  `fts64_children()`, `fts64_close()`, `fts64_open()`, `fts64_read()`,
  `fts64_set()`, `fts_children()`, `fts_close()`, `fts_open()`, `fts_read()`,
  `fts_set()`, `kill()`, `ptsname()` and `ptsname_r()`
- The internal function `path_resolution()` resolves the root directory if in
  chroot'ed environment and the resolved path is below the root directory
- Output some resolving shared object traces
- Look up the [ld.so(8)] cache file `/etc/ld.so.cache`
- Intercept the C2x [glibc] functions `__isoc23_sscanf()`, `__isoc23_strtol()`
  and `__isoc23_strtoul()` and forward them as is to the C2x symbol name if it
  exist or to the former symbol name `sscanf()`, `strtol()` and `strtoul()`
- Compile for [NetBSD] 9.3

### Changed

- Replace the environment variables `IAMROOT_LD_PRELOAD_<ldso>_<abi>` by
  `IAMROOT_PRELOAD_<ldso>_<abi>`
- Replace the environment variables `IAMROOT_LIBRARY_PATH_<ldso>_<abi>` and
  `IAMROOT_LIBRARY_PATH` by `IAMROOT_DEFLIB_<ldso>_<abi>` and `IAMROOT_DEFLIB`
- Replace the `ish` option `--library-path` by `--deflib`
- Do not lookup in `/usr/local` if [dynamic loader][ld.so(8)] is [glibc]

### Fixed

- Fix the missing stripping of the root directory in the functions `ftw()`,
  `nftw()` and `nftw64()`
- Fix the memory area overlapping if the root directory is stripped off
- Fix the interception of the `_time64` variant functions `__fstat64_time64()`,
  `__fstatat64_time64()`, `__lstat64_time64()` and `__stat64_time64()` ([glibc]
  only)

## [v11] - 2023-07-14

### Added

- Chroot to a RISC-V world; tested on [Arch Linux RISC-V] and [Alpine Linux]
- Output a warning if the `RPATH` or the `RUNPATH` contain dynamic string
  tokens
- Detects the default library path of the chroot'ed environment to get rid off
  the use of the environment variable `IAMROOT_LIBRARY_PATH` while chroot'ing
  in some 64-bit architectures GNU/Linux systems

### Changed

- Resolve manually the `DT_NEEDED` shared objects to preload them along with
  the `LD_PRELOAD` shared objects to preload (with the [ld.so(8)]) or to open
  them along with the shared object to open (with the functions `dlopen()` and
  `dlmopen()`), as described by [ld.so(8)] and [dlopen(3)], and at the
  exception of the cache file `/etc/ld.so.cache` (maintained by [ldconfig(8)])
  that is unchecked yet
- Use both ELF header and [dynamic loader][ld.so(8)] (`SONAME` and `ABI`) to
  detect the path to the library to preload
- Look up the directories specified in the deprecated dynamic section attribute
  `DT_RPATH` if the attribute `DT_RUNPATH` is unset only
- Do not lookup the default library paths if the flag `DF_1_NODEFLIB` is set
  in the dynamic entry `DT_FLAGS_1`
- Do not lookup the content of the environment variable `LD_LIBRARY_PATH`
  unless executable file runs in secure execution mode
- Ignore the command [mountpoint(1)] if `IAMROOT_EXEC_IGNORE` is unset

### Removed

- Remove the `iamroot-shell`'s option `--path-resolution-allow` and its
  environment variable `IAMROOT_PATH_RESOLUTION_ALLOW`
- Remove the `iamroot-shell`'s option `--debug-allow` and its environment
  variable `IAMROOT_DEBUG_ALLOW`
- Remove the two [exec][exec(3)] environment variables `IAMROOT_EXEC_LD_ARGV1`
  and `IAMROOT_EXEC_HASHBANG_ARGV1`

### Changed

- Increase the verbosity level for the opened, duplicated and fd traces to
  info

### Deprecated

- Replace the `iamroot-shell` environment variables `IAMROOT_PROFILE_FILE`,
  `IAMROOT_RC_FILE` and `IAMROOT_PRESERVE_ENV` by the environment variables
  `IAMROOT_SHELL_PROFILE_FILE`, `IAMROOT_SHELL_RC_FILE` and
  `IAMROOT_SHELL_PRESERVE_ENV`
- Replace the `iamroot-shell` script and its environment variables starting by
  `IAMROOT_SHELL` by the `ish` script and its environment variables starting by
  `ISH`, at the exception for `IAMROOT_SHELL` and `IAMROOT_SHELL_LVL` replaced
  by `ISH` and `ISHLVL`

### Fixed

- Fix the mispreloading of the needed libc.so shared object in `ish` (and
  libpthread.so on Linux) if extra shared objects are preloaded via the
  environment `LD_PRELOAD`

## [v10] - 2023-06-13

### Added

- Intercept the functions `ctermid()`, `fclose()`, `setfsuid()`, `setfsgid()`,
  `ttyname()`, `ttyname_r()`, `updwtmp()`, `updwtmpx()`, `utmpname()` and
  `utmpxname()`
- Compile for [OpenBSD] 7.2 and 7.3

### Changed

- The function `utimes()` does not call the AT-function `utimensat()` anymore
- Output the three-dots `...` for the functions `mkostemps()`, `mkostempsat()`,
  `scandir()`, `scandir64()`, `scandir_b()`, `scandirat()`
- Remove output the three-dots `...` for the function `umount()`
- Output the flags in hexadecimal for the functions `mount()`, `nmount()`,
  `umount2()` and `unmount()`
- Output the returned value for the functions
- Replace the environment variable `IAMROOT_PATH_RESOLUTION_WORKAROUND` by the
  environment variable `IAMROOT_PATH_RESOLUTION_WARNING_IGNORE` that disables
  the prepend of the root directory if the path to resolve contains it already
  via a regular expression

### Removed

- Remove functions `tmpnam()` and `tmpnam_r()`
- Remove the environment variable `IAMROOT_PATH_RESOLUTION_WORKAROUND`

### Fixed

- Fix the starting of `iamroot-shell` with a clean environment if the variable
  `IAMROOT_PRESERVE_ENV` is unset
- Fix the segmentation fault if the functions `__libc_start_main()`,
  `close_range()`, `get_current_dir_name()`, `getcwd()`, `getegid()`,
  `geteuid()`, `getgid()`, `getwd()` and `setuid()` return an error
- Fix the handling for NULL dir argument in the function `tempnam()` by
  defaulting to `P_tmpdir`
- Fix the missing stripping of the root directory in the function `tempnam()`
- Fix the freeing the NULL pointer in the function `canonicalize_file_name()`
- Fix the path resolution ignoration if the path is relative to the current
  working directory

### Security

- Fix invalid read if ignoring a path relative to the current working directory

## [v9] - 2023-05-06

### Added

- Intercept the [glibc] function `__openat_2()` again and create weak aliases
  for the [glibc] function `__openat64_2()`
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
  `close_range()`, `closefrom()`, `connect()`, `dup()`, `dup2()`, `dup3()`,
  `fdopen()`, `getpeername()`, `getresgid()`, `getresuid()`, `getsockname()`,
  `initgroups()`, `setregid()`, `setresgid()`, `setresuid()` and `setreuid()`
- Add the `iamroot-shell(1)`'s environment variables `IAMROOT_SHELL_COMMAND`,
  `IAMROOT_SHELL_GID`, `IAMROOT_SHELL_UID` and `IAMROOT_SHELL_USER`
- Add the `iamroot-shell`'s option `--user` and its environment variable
  `IAMROOT_USER` to usurp the user identity
- Add the `iamroot-shell`'s option `--shell` and its environment variable
  `IAMROOT_SHELL` to set the shell interpreter to use

### Removed

- Remove function `mq_open()`
- Remove output the architecture, the libc and the pid on level 6 and above
- Remove output the `root` directory for the functions `__fpathperror()`,
  `__fpathperror2()`, `__pathperror()` and `__pathperror2()`

### Changed

- Output the directory fd for the functions `name_to_handle_at()` and
  `scandirat()`
- Use `/lib64:/usr/local/lib64:/usr/lib64` library path for both [glibc]
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
  `PATH`, `HOME` and `SHELL`)
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
- The environment variable `USER` is now set to the usurper user login name
- Output the opened, duplicated and closed fd

### Deprecated

- Replace the `iamroot-shell(1)` environment variable `IAMROOTLVL` by
  the environment variable `IAMROOT_SHELL_LVL`
- Replace the `iamroot-shell(1)` option `--chroot` by the option `--root`

### Fixed

- Fix the output of the open flags for the function `renameat2()`
- Fix the alteration of the `errno` error in the internal function
  `__procfdreadlink()` and the functions `getegid()`, `geteuid()`, `getgid()`
  and `getuid()`
- Fix the loading of the user ownership from extended attributes of the
  resolved paths in the function `fchownat()`
- Fix the missing stripping of the root directory in the function
  `canonicalize_file_name()`
- Fix the truncating of symlinks read from a root directory if the buffer is
  too small to contain the whole symlink
- Fix the handling for empty path if dfd is not `AT_FDCWD`
- Fix the calling to the next symbol with the resolved path for the function
  `ftok()`
- Fix the handling for path argument without slash in the function `catopen()`
  by opening the catalog files from the environment variable `NLSPATH`
- Fix the following symlink behaviour for the function `name_to_handle_at()` by
  adding manually the `AT` flag `AT_SYMLINK_NOFOLLOW` to `path_resolution()`

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
- Output the parameters even in chroot'ed environment  for the functions
  `execveat()`, `execve()` and `posix_spawn()`
- Output a warning if the interpreter is not handled
- Support the environment variables `argv0`, `LD_LIBRARY_PATH` and `LD_PRELOAD`
  while running a generic ELF dynamic loader
- Remove the library path prefix `LD_` from the environment variable
  `IAMROOT_LD_LIBRARY_PATH` and the prefix `-ld` from the script option
  `--ld-library-path`
- The functions `__fxstat()`, `__fxstat64()`, `__lxstat()`, `__lxstat64()`,
  `__xstat()`, `__xstat64()`, `fstat()`, `fstat64()`, `lstat()`, `lstat64()`,
  `stat()` and `stat64()` call the functions `__fxstatat()`, `__fxstatat64()`,
  `fstatat()` and `fstatat64()`
- The functions `fchmod()` and `fchown()` call the functions `fchownat()` and
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
  not support the option `--argv0` before 2.33
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
  by the [dynamic loader][ld.so(8)] in the chroot'ed environment; the two
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

- Run the [dynamic loader][ld.so(8)] directly
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

[Adélie Linux]: https://www.adelielinux.org/
[Alpine Linux]: https://alpinelinux.org/
[Arch Linux 32]: https://archlinux32.org/
[Arch Linux ARM]: https://archlinuxarm.org/
[Arch Linux Power]: https://archlinuxpower.org/
[Arch Linux RISC-V]: https://archriscv.felixc.at/
[Debian ARM 64 Port]: https://wiki.debian.org/Arm64Port
[Debian ARM EABI Port]: https://wiki.debian.org/ArmEabiPort
[Debian ARM hard-float Port]: https://wiki.debian.org/ArmHardFloatPort
[Debian MIPS Port]: https://wiki.debian.org/MIPSPort
[Debian PowerPC Port]: https://www.debian.org/ports/powerpc/
[Debian PowerPC64 Port]: https://wiki.debian.org/ppc64el
[Debian RISC-V 64 Port]: https://wiki.debian.org/Ports/riscv64
[Debian S/390 Port]: https://www.debian.org/ports/s390/
[Debian i386 Port]: https://www.debian.org/ports/i386/
[Debian multiarch]: https://wiki.debian.org/Multiarch
[Debian]: https://www.debian.org/
[Fedora 30]: https://docs.fedoraproject.org/en-US/releases/f30/
[Fedora ARM]: https://arm.fedoraproject.org/
[Fedora]: https://getfedora.org/
[FreeBSD]: https://www.freebsd.org/
[Jim Tcl]: https://jim.tcl.tk/index.html/doc/www/www/index.html
[Leap]: https://www.opensuse.org/#Leap
[Manjaro]: https://manjaro.org/
[Mobian]: https://mobian-project.org/
[NO_COLOR]: https://no-color.org/
[NetBSD]: https://www.netbsd.org/
[OpenBSD]: https://www.openbsd.org/
[Tumbleweed]: https://www.opensuse.org/#Tumbleweed
[Ubuntu]: https://ubuntu.com/
[__nss_files_fopen()]: https://sourceware.org/git/?p=glibc.git;a=blob;f=nss/nss_files_fopen.c;h=594e4216578766e4534c44dc6c75283d5d1a20fe;hb=299210c1fa67e2dfb564475986fce11cd33db9ad
[alpine-make-rootfs]: https://github.com/alpinelinux/alpine-make-rootfs/blob/master/README.adoc
[busybox(1)]: https://linux.die.net/man/1/busybox
[capabilities(7)]: https://linux.die.net/man/7/capabilities
[debootstrap(8)]: https://linux.die.net/man/8/debootstrap
[dlopen(3)]: https://linux.die.net/man/3/dlopen
[dnf(8)]: https://dnf.readthedocs.io/en/latest/command_ref.html
[exec(3)]: https://linux.die.net/man/3/exec
[glibc 2.29]: https://sourceware.org/glibc/wiki/Release/2.29
[glibc]: https://www.gnu.org/software/libc/
[ld.so(8)]: https://linux.die.net/man/8/ld.so
[ldconfig(8)]: https://linux.die.net/man/8/ldconfig
[mmdebstrap(1)]: https://manpages.debian.org/testing/mmdebstrap/mmdebstrap.1.en.html
[mountpoint(1)]: https://linux.die.net/man/1/mountpoint
[musl]: https://www.musl-libc.org/
[openSUSE]: https://www.opensuse.org/
[pacstrap(8)]: https://man.archlinux.org/man/pacstrap.8
[passwd(1)]: https://linux.die.net/man/1/passwd
[su(1)]: https://linux.die.net/man/1/su
[systemd-sysusers(8)]: https://www.freedesktop.org/software/systemd/man/systemd-sysusers.html
[unreleased]: https://github.com/gportay/iamroot/compare/v23...master
[v1]: https://github.com/gportay/iamroot/releases/tag/v1
[v2]: https://github.com/gportay/iamroot/compare/v1...v2
[v3]: https://github.com/gportay/iamroot/compare/v2...v3
[v4]: https://github.com/gportay/iamroot/compare/v3...v4
[v5]: https://github.com/gportay/iamroot/compare/v4...v5
[v6]: https://github.com/gportay/iamroot/compare/v5...v6
[v7]: https://github.com/gportay/iamroot/compare/v6...v7
[v8]: https://github.com/gportay/iamroot/compare/v7...v8
[v9]: https://github.com/gportay/iamroot/compare/v8...v9
[v10]: https://github.com/gportay/iamroot/compare/v9...v10
[v11]: https://github.com/gportay/iamroot/compare/v10...v11
[v12]: https://github.com/gportay/iamroot/compare/v11...v12
[v13]: https://github.com/gportay/iamroot/compare/v12...v13
[v14]: https://github.com/gportay/iamroot/compare/v13...v14
[v15]: https://github.com/gportay/iamroot/compare/v14...v15
[v16]: https://github.com/gportay/iamroot/compare/v15...v16
[v17]: https://github.com/gportay/iamroot/compare/v16...v17
[v18]: https://github.com/gportay/iamroot/compare/v17...v18
[v19]: https://github.com/gportay/iamroot/compare/v18...v19
[v20]: https://github.com/gportay/iamroot/compare/v19...v20
[v21]: https://github.com/gportay/iamroot/compare/v20...v21
[v22]: https://github.com/gportay/iamroot/compare/v21...v22
[v23]: https://github.com/gportay/iamroot/compare/v22...v23
[zypper(8)]: https://en.opensuse.org/SDB:Zypper_manual_(plain)
