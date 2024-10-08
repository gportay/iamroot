/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _IAMROOT_H_
#define _IAMROOT_H_

#define constructor __attribute__((constructor,__visibility__("hidden")))
#define destructor __attribute__((destructor,__visibility__("hidden")))

/*
 * Stolen from musl (src/include/features.h)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#define weak __attribute__((__weak__))
#define hidden __attribute__((__visibility__("hidden")))
#define weak_alias(old, new) \
	extern __typeof(old) new __attribute__((__weak__, __alias__(#old)))

/*
 * Stolen from musl (src/internal/syscall_ret.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static inline long __syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		errno = -r;
		return -1;
	}
	return r;
}

/*
 * Stolen from systemd (src/fundamental/string-util-fundamental.h)
 *
 * SPDX-FileCopyrightText: The systemd Contributors
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#define streq(a,b) (strcmp((a),(b)) == 0)
#define strneq(a, b, n) (strncmp((a), (b), (n)) == 0)

#ifdef __cplusplus
extern "C" {
#endif

#define __xstr(s) __str(s)
#define __str(s) #s

#define __min(a,b) \
	({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	   _a < _b ? _a : _b; })

#define __max(a,b) \
	({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	   _a > _b ? _a : _b; })

#define __set_errno(e, r) \
	({ errno = (e); \
	   (r); })

#define __set_errno_and_perror(e, r) \
	({ errno = (e); \
	   perror(__func__); \
	   (r); })

#define _strncpy(s1, s2, n1) \
	({ const int l = (n1)-1; \
	   strncpy((s1), (s2), l); \
	   (s1)[l] = 0; \
	   (char *)(s1); })
#define _strncat(s1, s2, n1) \
	({ const int l = (n1)-1; \
	   strncat((s1), (s2), l); \
	   (s1)[l] = 0; \
	   (char *)(s1); })
#define __strchrnul strchrnul
#define __strlen(s) (strnlen((s), PATH_MAX))
#define __strnlen(s) (strnlen((s), sizeof((s))-1))
#define __strlcmp(s1, s2) (strncmp((s1), (s2), __strlen((s2))))
#define __strncmp(s1, s2) (strncmp((s1), (s2), sizeof((s2))-1))
#define __strlcpy(s1, s2) \
	({ const int l = __strlen((s2)); \
	   strncpy((s1), (s2), l); \
	   (s1)[l] = 0; \
	   (char *)(s1); })
#define __strncpy(s1, s2) \
	({ const int l = sizeof((s1))-1; \
	   strncpy((s1), (s2), l); \
	   (s1)[l] = 0; \
	   (char *)(s1); })
#define __strncat(s1, s2) \
	({ const int l = sizeof((s1))-1; \
	   strncat((s1), (s2), l); \
	   (s1)[l] = 0; \
	   (char *)(s1); })
#define __strleq(s1, s2) (__strlcmp((s1), (s2)) == 0)
#define __strneq(s1, s2) (__strncmp((s1), (s2)) == 0)
#define __snprintf(s, fmt, ...) _snprintf((s), sizeof((s))-1, fmt, __VA_ARGS__)

#ifndef _STAT_VER
#if defined(__arm__) || defined(__powerpc__) || defined(__mips__) || \
    defined(__i386__)
#define _STAT_VER 3
#else
#define _STAT_VER 0
#endif
#endif

#define PATH_RESOLUTION_NOMAGICLINKS 0x01
#define PATH_RESOLUTION_NOIGNORE     0x02
#define PATH_RESOLUTION_NOWALKALONG  0x04

#ifdef __linux__
/*
 * According to man open(2):
 *
 * The mode argument specifies the file mode bits to be applied when a new file
 * is created. If neither O_CREAT nor O_TMPFILE is specified in flags, then mode
 * is ignored (and can thus be specified as 0, or simply omitted). The mode
 * argument must be supplied if O_CREAT or O_TMPFILE is specified in flags; if
 * it is not supplied, some arbitrary bytes from the stack will be applied as
 * the file mode.
 */
#define __has_create_flag(oflags) ((oflags) & O_CREAT)
#ifdef O_TMPFILE
#define __has_tmpfile_flag(oflags) ((oflags) & O_TMPFILE)
#else
#define __has_tmpfile_flag(oflags) (0)
#endif
#define __needs_mode(oflags) (__has_create_flag((oflags)) || __has_tmpfile_flag((oflags)))

int __secure();
#endif

#ifdef __OpenBSD__
char *strchrnul(const char *, int);
#endif

#ifndef __linux__
#define __environ environ

extern char **environ;
#else
extern char **__environ;

#define IAMROOT_XATTRS_PREFIX          "user.iamroot."
#define IAMROOT_XATTRS_MODE            IAMROOT_XATTRS_PREFIX "mode"
#define IAMROOT_XATTRS_UID             IAMROOT_XATTRS_PREFIX "uid"
#define IAMROOT_XATTRS_GID             IAMROOT_XATTRS_PREFIX "gid"
#define IAMROOT_XATTRS_PATH_RESOLUTION IAMROOT_XATTRS_PREFIX "path-resolution"
#endif

#if defined __FreeBSD__ || __NetBSD__
#define IAMROOT_EXTATTR_PREFIX "iamroot."
#define IAMROOT_EXTATTR_MODE            IAMROOT_EXTATTR_PREFIX "mode"
#define IAMROOT_EXTATTR_UID             IAMROOT_EXTATTR_PREFIX "uid"
#define IAMROOT_EXTATTR_GID             IAMROOT_EXTATTR_PREFIX "gid"
#define IAMROOT_EXTATTR_PATH_RESOLUTION IAMROOT_EXTATTR_PREFIX "path-resolution"
#endif

char **_resetenv(char **);
char *_getenv(const char *);
int _clearenv();
int _putenv(char *);
int _setenv(const char *, const char *, int);
int _unsetenv(const char *);
int _snprintf(char *, size_t, const char *, ...) __attribute__((format(printf,3,4)));

int __fis_symlinkat(int, const char *, int);
int __fis_symlink(int);
int __is_symlink(const char *);
int __fis_directoryat(int, const char *, int);
int __fis_directory(int);
int __is_directory(const char *);
int __fis_fileat(int, const char *, int);
int __fis_file(int);
int __is_file(const char *);
const char *__basename(const char *);
int __can_exec(const char *);
char *__getenv(const char *);
int __setenv(const char *, const char *, int);

int __execve(const char *, char * const [], char * const []);
int __exec_ignored(const char *);
int __is_suid(const char *);
int __ldso(const char *, char * const [], char *[], char *, size_t, off_t);
int __exec_sh(const char *, char * const *, char *[], char *, size_t);
ssize_t __interpreter_script_hashbang(const char *, char *, size_t, off_t);
int __interpreter_script(const char *, char * const [], char *, size_t, off_t,
			 char *[]);

char *__path_sanitize(char *, size_t);
char *__fpath(int);
char *__fpath2(int);
ssize_t fpath(int, char *, size_t);
int __path_ignored(int, const char *);
ssize_t path_resolution(int, const char *, char *, size_t, int);
ssize_t path_resolution2(int, const char *, char *, size_t, int, int);
char *__getpath(int, const char *, int);
ssize_t __host_path_access(const char *, int, const char *, char *, size_t,
			   off_t);
ssize_t __path_access(const char *, int, const char *, char *, size_t);
int __path_iterate(const char *, int (*)(const char *, void *), void *);
int __group_iterate(const char *, int (*)(const char *, void *), void *);
int __dir_iterate(const char *, int (*)(const char *, const char *, void *),
		  void *);
int __strtofd(const char *, char **);

#if defined __linux__ || defined __NetBSD__
void __procfdname(char *, unsigned);
#endif

char *__getroot();
#if defined __linux__ || defined __FreeBSD__ || defined __NetBSD__
const char *__getexe();
#endif

const char *__getfd(int);
#define __unsetfd(fd) __setfd((fd), NULL)
int __setfd(int, const char *);
int __execfd();
const char *__execfn();
int __setrootdir(const char *);
const char *__getrootdir();
int __chrootdir(const char *);
int __inchroot();
const char *__skiprootdir(const char *);
char *__striprootdir(char *);

int __getno_color();
int __getcolor();

#define DEBUG_FILENO __getdebug_fd()

int __getdebug();
int __getdebug_fd();
int __verbosef(int, const char *, const char *, ...) __attribute__((format(printf,3,4)));

#if !defined(NVERBOSE)
#define __debug(fmt, ...) __verbosef(3, __func__, fmt, __VA_ARGS__)
#define __info(fmt, ...) __verbosef(2, __func__, fmt, __VA_ARGS__)
#define __notice(fmt, ...) __verbosef(1, __func__, fmt, __VA_ARGS__)
#define __warning(fmt, ...) __verbosef(0, __func__, fmt, __VA_ARGS__)
#define __debug_execve(argv, envp) __verbose_execve(3, argv, envp)
#define __info_execve(argv, envp) __verbose_execve(2, argv, envp)
#define __notice_execve(argv, envp) __verbose_execve(1, argv, envp)
#define __warning_execve(argv, envp) __verbose_execve(0, argv, envp)
void __verbose_execve(int, char * const[], char * const[]);
#else
#define __debug(fmt, ...) {}
#define __info(fmt, ...) {}
#define __notice(fmt, ...) {}
#define __warning(fmt, ...) {}
#define __debug_execve(argv, envp) {}
#define __info_execve(argv, envp) {}
#define __notice_execve(argv, envp) {}
#define __warning_execve(argv, envp) {}
#define __verbose_execve(lvl, argv, envp) {}
#endif

#define __note_if_not_preloading_libiamroot_and_ensure_preloading() \
	({ const int errno_save = errno; \
	   if (__preload_libiamroot() == 0) \
	     __notice("%s: preloading libraries '%s'!\n", __func__, _getenv("PRELOAD")); \
	   errno = errno_save; \
	   })

#define __note_if_envp_is_not_environ(envp) \
	({ if (!(envp)) \
	     __notice("%s: envp is %p!\n", __func__, (envp)); \
	   if (!(envp) && (envp) != (__environ)) \
	     __notice("%s: envp and environ differs!\n", __func__); \
	   ((envp) != (__environ)); \
	   })

#define __warn_and_set_user_mode(path, mode, user_mode) \
	({ if (((mode) & (user_mode)) != (user_mode)) { \
	     __info("%s: %s: Insuffisant user mode 0%03o!\n", __func__, (path), (mode)); \
	     (mode) |= (user_mode); \
	   } \
	   if ((mode) & S_ISUID) { \
	     __info("%s: %s: SUID bit 0%04o!\n", __func__, (path), (mode)); \
	     (mode) &= ~S_ISUID; \
	   } \
	   if ((mode) & S_ISGID) { \
	     __info("%s: %s: SGID bit 0%04o!\n", __func__, (path), (mode)); \
	     (mode) &= ~S_ISGID; \
	   } })

#define __fwarn_and_set_user_mode(fd, mode, user_mode) \
	({ if (((mode) & (user_mode)) != (user_mode)) { \
	     __info("%s: %i <-> %s: Insuffisant user mode 0%03o!\n", __func__, (fd), __fpath((fd)), (mode)); \
	     (mode) |= (user_mode); \
	   } \
	   if ((mode) & S_ISUID) { \
	     __info("%s: %i <-> %s: SUID bit 0%04o!\n", __func__, (fd), __fpath((fd)), (mode)); \
	     (mode) &= ~S_ISUID; \
	   } \
	   if ((mode)& S_ISGID) { \
	     __info("%s: %i <-> %s: SGID bit 0%04o!\n", __func__, (fd), __fpath((fd)), (mode)); \
	     (mode) &= ~S_ISGID; \
	   } })

#define __fwarn_and_set_user_modeat(fd, path, mode, flags, user_mode) \
	({ if (((mode) & (user_mode)) != (user_mode)) { \
	     __info("%s: %i/%s: Insuffisant user mode 0%03o!\n", __func__, (fd), (path), (mode)); \
	     (mode) |= (user_mode); \
	   } \
	   if ((mode) & S_ISUID) { \
	     __info("%s: %i/%s: SUID bit 0%04o!\n", __func__, (fd), (path), (mode)); \
	     (mode) &= ~S_ISUID; \
	   } \
	   if ((mode)& S_ISGID) { \
	     __info("%s: %i/%s: SGID bit 0%04o!\n", __func__, (fd), (path), (mode)); \
	     (mode) &= ~S_ISGID; \
	   } })

#define __warn_if_insuffisant_user_mode(path, mode) \
	({ if (__is_directory((path)) > 0) { \
	     __warn_and_set_user_mode((path), (mode), 0700); \
	   } else { \
	     __warn_and_set_user_mode((path), (mode), 0600); \
	   } })

#define __fwarn_if_insuffisant_user_mode(fd, mode) \
	({ if (__fis_directory((fd)) > 0) { \
	     __fwarn_and_set_user_mode((fd), (mode), 0700); \
	   } else { \
	     __fwarn_and_set_user_mode((fd), (mode), 0600); \
	   } })

#define __fwarn_if_insuffisant_user_modeat(fd, path, mode, flags) \
	({ if (__fis_directoryat((fd), (path), (flags)) > 0) { \
	     __fwarn_and_set_user_modeat((fd), (path), (mode), (flags), 0700); \
	   } else { \
	     __fwarn_and_set_user_modeat((fd), (path), (mode), (flags), 0600); \
	   } })

#define __warn_and_set_umask(mask, user_mask) \
	({ if ((mask) & (user_mask)) { \
	     __info("%s: Too restrictive umask 0%03o!\n", __func__, (mask)); \
	     (mask) &= ~(user_mask); \
	   } })

#define __warn_if_too_restrictive_umask(mask) \
	({ __warn_and_set_umask((mask), 0400); })

#ifdef __linux__
extern ssize_t next_fgetxattr(int, const char *, void *, size_t);
extern int next_fsetxattr(int, const char *, const void *, size_t, int);
extern int next_fremovexattr(int, const char *);
extern ssize_t next_lgetxattr(const char *, const char *, void *, size_t);
extern int next_lsetxattr(const char *, const char *, const void *, size_t,
			  int);
extern int next_lremovexattr(const char *, const char *);

#define __get_mode(path) \
	({ const int errno_save = errno; \
	   mode_t m = (mode_t)-1; \
	   if (next_lgetxattr((path), IAMROOT_XATTRS_MODE, &m, sizeof(m)) != sizeof(m)) \
		m = (mode_t)-1; \
	   __set_errno(errno_save, m); \
	})

#define __set_mode(path, oldmode, mode) \
	({ const int errno_save = errno; \
	   if ((oldmode) == (mode)) { \
	     next_lremovexattr((path), IAMROOT_XATTRS_MODE); \
	   } else { \
	     next_lsetxattr((path), IAMROOT_XATTRS_MODE, &(oldmode), sizeof((oldmode)), 0); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __fget_mode(fd) \
	({ const int errno_save = errno; \
	   mode_t m = (mode_t)-1; \
	   if (next_fgetxattr((fd), IAMROOT_XATTRS_MODE, &m, sizeof(m)) != sizeof(m)) \
		m = (mode_t)-1; \
	   __set_errno(errno_save, m); \
	})

#define __fset_mode(fd, oldmode, mode) \
	({ const int errno_save = errno; \
	   if ((oldmode) == (mode)) { \
	     next_fremovexattr((fd), IAMROOT_XATTRS_MODE); \
	   } else { \
	     next_fsetxattr((fd), IAMROOT_XATTRS_MODE, &(oldmode), sizeof((oldmode)), 0); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __get_uid(path) \
	({ const int errno_save = errno; \
	   uid_t u = (uid_t)-1; \
	   if (next_lgetxattr((path), IAMROOT_XATTRS_UID, &u, sizeof(u)) != sizeof(u)) \
		u = (uid_t)-1; \
	   __set_errno(errno_save, u); \
	})

#define __set_uid(path, olduid, uid) \
	({ const int errno_save = errno; \
	   if ((olduid) == (uid)) { \
	     next_lremovexattr((path), IAMROOT_XATTRS_UID); \
	   } else { \
	     next_lsetxattr((path), IAMROOT_XATTRS_UID, &(olduid), sizeof((olduid)), 0); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __fget_uid(fd) \
	({ const int errno_save = errno; \
	   uid_t u = (uid_t)-1; \
	   if (next_fgetxattr((fd), IAMROOT_XATTRS_UID, &u, sizeof(u)) != sizeof(u)) \
		u = (uid_t)-1; \
	   __set_errno(errno_save, u); \
	})

#define __fset_uid(fd, olduid, uid) \
	({ const int errno_save = errno; \
	   if ((olduid) == (uid)) { \
	     next_fremovexattr((fd), IAMROOT_XATTRS_UID); \
	   } else { \
	     next_fsetxattr((fd), IAMROOT_XATTRS_UID, &(olduid), sizeof((olduid)), 0); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __get_gid(path) \
	({ const int errno_save = errno; \
	   gid_t g = (gid_t)-1; \
	   if (next_lgetxattr((path), IAMROOT_XATTRS_GID, &g, sizeof(g)) != sizeof(g)) \
		g = (gid_t)-1; \
	   __set_errno(errno_save, g); \
	})

#define __set_gid(path, oldgid, gid) \
	({ const int errno_save = errno; \
	   if ((oldgid) == (gid)) { \
	     next_lremovexattr((path), IAMROOT_XATTRS_GID); \
	   } else { \
	     next_lsetxattr((path), IAMROOT_XATTRS_GID, &(oldgid), sizeof((oldgid)), 0); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __fget_gid(fd) \
	({ const int errno_save = errno; \
	   gid_t g = (gid_t)-1; \
	   if (next_fgetxattr((fd), IAMROOT_XATTRS_GID, &g, sizeof(g)) != sizeof(g)) \
		g = (gid_t)-1; \
	   __set_errno(errno_save, g); \
	})

#define __fset_gid(fd, oldgid, gid) \
	({ const int errno_save = errno; \
	   if ((oldgid) == (gid)) { \
	     next_fremovexattr((fd), IAMROOT_XATTRS_GID); \
	   } else { \
	     next_fsetxattr((fd), IAMROOT_XATTRS_GID, &(oldgid), sizeof((oldgid)), 0); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __get_path_resolution(path, data, datasiz) \
	({ const int errno_save = errno; \
	   const int r = next_lgetxattr((path), IAMROOT_XATTRS_PATH_RESOLUTION, (data), (datasiz)-1); \
	   __set_errno(errno_save, r); \
	})

#define __fget_path_resolution(fd, data, datasiz) \
	({ const int errno_save = errno; \
	   const int r = next_fgetxattr((fd), IAMROOT_XATTRS_PATH_RESOLUTION, (data), (datasiz)-1); \
	   __set_errno(errno_save, r); \
	})

#define __unset_path_resolution(path) \
	({ const int errno_save = errno; \
	   const int r = next_lremovexattr((path), IAMROOT_XATTRS_PATH_RESOLUTION); \
	   __set_errno(errno_save, r); \
	})

#define __set_path_resolution(path, data) \
	({ const int errno_save = errno; \
	   int r; \
	   if (!data || !*data) { \
	     r = __unset_path_resolution((path)); \
	   } else { \
	     r = next_lsetxattr((path), IAMROOT_XATTRS_PATH_RESOLUTION, (data), __strlen((data))+1, 0); \
	   } \
	   __set_errno(errno_save, r); \
	})
#endif

#if defined __FreeBSD__ || defined __NetBSD__
extern ssize_t next_extattr_get_fd(int, int, const char *, void *, size_t);
#ifdef __NetBSD__
extern int next_extattr_set_fd(int, int, const char *, const void *, size_t);
#else
extern ssize_t next_extattr_set_fd(int, int, const char *, const void *,
				   size_t);
#endif
extern int next_extattr_delete_fd(int, int, const char *);
extern ssize_t next_extattr_get_link(const char *, int, const char *, void *,
				     size_t);
#ifdef __NetBSD__
extern int next_extattr_set_link(const char *, int, const char *,
				 const void *, size_t);
#else
extern ssize_t next_extattr_set_link(const char *, int, const char *,
				     const void *, size_t);
#endif
extern int next_extattr_delete_link(const char *, int, const char *);

#ifndef EXTATTR_NAMESPACE_USER
#define EXTATTR_NAMESPACE_USER 0x00000001
#endif

#define __get_mode(path) \
	({ const int errno_save = errno; \
	   mode_t m = (mode_t)-1; \
	   if (next_extattr_get_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_MODE, &m, sizeof(m)) != sizeof(m)) \
		m = (mode_t)-1; \
	   __set_errno(errno_save, m); \
	})

#define __set_mode(path, oldmode, mode) \
	({ const int errno_save = errno; \
	   if ((oldmode) == (mode)) { \
	     next_extattr_delete_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_MODE); \
	   } else { \
	     next_extattr_set_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_MODE, &(oldmode), sizeof((oldmode))); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __fget_mode(fd) \
	({ const int errno_save = errno; \
	   mode_t m = (mode_t)-1; \
	   if (next_extattr_set_fd((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_MODE, &m, sizeof(m)) != sizeof(m)) \
		m = (mode_t)-1; \
	   __set_errno(errno_save, m); \
	})

#define __fset_mode(fd, oldmode, mode) \
	({ const int errno_save = errno; \
	   if ((oldmode) == (mode)) { \
	     next_extattr_delete_link((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_MODE); \
	   } else { \
	     next_extattr_set_link((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_MODE, &(oldmode), sizeof((oldmode)), 0); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __get_uid(path) \
	({ const int errno_save = errno; \
	   uid_t u = (uid_t)-1; \
	   if (next_extattr_get_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_UID, &u, sizeof(u)) != sizeof(u)) \
		u = (uid_t)-1; \
	   __set_errno(errno_save, u); \
	})

#define __set_uid(path, olduid, uid) \
	({ const int errno_save = errno; \
	   if ((olduid) == (uid)) { \
	     next_extattr_delete_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_UID); \
	   } else { \
	     next_extattr_set_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_UID, &(olduid), sizeof((olduid))); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __fget_uid(fd) \
	({ const int errno_save = errno; \
	   uid_t u = (uid_t)-1; \
	   if (next_extattr_get_fd((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_UID, &u, sizeof(u)) != sizeof(u)) \
		u = (uid_t)-1; \
	   __set_errno(errno_save, u); \
	})

#define __fset_uid(fd, olduid, uid) \
	({ const int errno_save = errno; \
	   if ((olduid) == (uid)) { \
	     next_extattr_delete_fd((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_UID); \
	   } else { \
	     next_extattr_set_fd((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_UID, &(olduid), sizeof((olduid))); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __get_gid(path) \
	({ const int errno_save = errno; \
	   gid_t g = (gid_t)-1; \
	   if (next_extattr_get_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_MODE, &g, sizeof(g)) != sizeof(g)) \
		g = (gid_t)-1; \
	   __set_errno(errno_save, g); \
	})

#define __set_gid(path, oldgid, gid) \
	({ const int errno_save = errno; \
	   if ((oldgid) == (gid)) { \
	     next_extattr_delete_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_GID); \
	   } else { \
	     next_extattr_set_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_GID, &(oldgid), sizeof((oldgid))); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __fget_gid(fd) \
	({ const int errno_save = errno; \
	   gid_t g = (gid_t)-1; \
	   if (next_extattr_get_fd((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_MODE, &g, sizeof(g)) != sizeof(g)) \
		g = (gid_t)-1; \
	   __set_errno(errno_save, g); \
	})

#define __fset_gid(fd, oldgid, gid) \
	({ const int errno_save = errno; \
	   if ((oldgid) == (gid)) { \
	     next_extattr_delete_fd((fd), EXTATTR_NAMESPACE_USER, \
				    IAMROOT_EXTATTR_GID); \
	   } else { \
	     next_extattr_set_fd((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_GID, &(oldgid), sizeof((oldgid))); \
	   } \
	   __set_errno(errno_save, 0); \
	})

#define __get_path_resolution(path, data, datasiz) \
	({ const int errno_save = errno; \
	   const int r = next_extattr_get_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_PATH_RESOLUTION, (data), (datasiz)-1); \
	   __set_errno(errno_save, r); \
	})

#define __fget_path_resolution(fd, data, datasiz) \
	({ const int errno_save = errno; \
	   const int r = next_extattr_get_fd((fd), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_PATH_RESOLUTION, (data), (datasiz)-1); \
	   __set_errno(errno_save, r); \
	})

#define __unset_path_resolution(path) \
	({ const int errno_save = errno; \
	   const int r = next_extattr_delete_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_PATH_RESOLUTION); \
	   __set_errno(errno_save, r); \
	})

#define __set_path_resolution(path, data) \
	({ const int errno_save = errno; \
	   int r; \
	   if (!data || !*data) { \
	     r = __unset_path_resolution((path)); \
	   } else { \
	     r = next_extattr_set_link((path), EXTATTR_NAMESPACE_USER, IAMROOT_EXTATTR_PATH_RESOLUTION, (data), __strlen((data))+1); \
	   } \
	   __set_errno(errno_save, r); \
	})
#endif

#if defined __linux__ || defined __FreeBSD__ || defined __NetBSD__
#define __st_mode(path, statbuf) \
	({ mode_t m = __get_mode((path)); \
	   if (m != (mode_t)-1) { \
	     statbuf->st_mode = (statbuf->st_mode & S_IFMT) | m; \
	   } })

#define __fst_mode(fd, statbuf) \
	({ mode_t m = __fget_mode((fd)); \
	   if (m != (mode_t)-1) { \
	     statbuf->st_mode = (statbuf->st_mode & S_IFMT) | m; \
	   } })

#define __stx_mode(path, statxbuf) \
	({ mode_t m = __get_mode((path)); \
	   if (m != (mode_t)-1) { \
	     statxbuf->stx_mode = (statxbuf->stx_mode & S_IFMT) | m; \
	   } })

#define __st_uid(path, statbuf) \
	({ uid_t u = __get_uid((path)); \
	   if (u != (uid_t)-1) { \
	     statbuf->st_uid = u; \
	   } else { \
	     statbuf->st_uid = 0; \
	   } })

#define __fst_uid(fd, statbuf) \
	({ uid_t u = __fget_uid((fd)); \
	   if (u != (uid_t)-1) { \
	     statbuf->st_uid = u; \
	   } else { \
	     statbuf->st_uid = 0; \
	   } })

#define __stx_uid(path, statxbuf) \
	({ uid_t u = __get_uid((path)); \
	   if (u != (uid_t)-1) { \
	     statxbuf->stx_uid = u; \
	   } else { \
	     statxbuf->stx_uid = 0; \
	   } })

#define __st_gid(path, statbuf) \
	({ gid_t g = __get_gid((path)); \
	   if (g != (gid_t)-1) { \
	     statbuf->st_gid = g; \
	   } else { \
	     statbuf->st_gid = 0; \
	   } })

#define __fst_gid(fd, statbuf) \
	({ gid_t g = __fget_gid((fd)); \
	   if (g != (gid_t)-1) { \
	     statbuf->st_gid = g; \
	   } else { \
	     statbuf->st_gid = 0; \
	   } })

#define __stx_gid(path, statxbuf) \
	({ gid_t g = __get_gid((path)); \
	   if (g != (gid_t)-1) { \
	     statxbuf->stx_gid = g; \
	   } else { \
	     statxbuf->stx_gid = 0; \
	   } })
#else
#define __get_mode(path) ({ (mode_t)-1; })
#define __set_mode(path, oldmode, mode) {}
#define __fget_mode(fd) ({ (mode_t)-1; })
#define __fset_mode(fd, oldmode, mode) {}
#define __get_uid(path) ({ (uid_t)-1; })
#define __set_uid(path, olduid, uid) {}
#define __fget_uid(fd) ({ (uid_t)-1; })
#define __fset_uid(fd, olduid, uid) {}
#define __get_gid(path) ({ (gid_t)-1; })
#define __set_gid(path, oldgid, gid) {}
#define __fget_gid(fd) ({ (gid_t)-1; })
#define __fset_gid(fd, oldgid, gid) {}
#define __st_mode(path, statbuf) {}
#define __fst_mode(fd, statbuf) {}
#define __st_uid(path, statbuf) {}
#define __fst_uid(fd, statbuf) {}
#define __st_gid(path, statbuf) {}
#define __fst_gid(fd, statbuf) {}
#endif

#define __dlperror(s) __info("%s: %s\n", s, dlerror())

#define __dl_set_errno_and_perror(e, r) \
	({ __dlperror(__func__); \
	   __set_errno(e, (r)); })

#ifndef __NetBSD__
int __dlopen_needed(const char *, int);
#endif

int munmap(void *, size_t);
int close(int);
void perror(const char *);
static inline void __munmap(void *addr, size_t len)
{
	const int errno_save = errno;

	if (munmap(addr, len))
		perror("munmap");
	errno = errno_save;
}
static inline void __close(int fd)
{
	extern int errno;
	const int errno_save = errno;

	if (close(fd))
		perror("close");
	errno = errno_save;
}

const char *__path();

int __preload_libiamroot();

int __is_ldso(const char *);

ssize_t __dl_access(const char *, int, char *, size_t);

int __elf_has_interp(int);

ssize_t __ldso_cache(const char *, char *, size_t);

/*
 * glibc considers the kernel headers define a wrong value for ARG_MAX and
 * undefines it. Let's redefine it using _POSIX_ARG_MAX.
 */
#ifndef ARG_MAX
#define ARG_MAX _POSIX_ARG_MAX
#endif

#ifdef __cplusplus
}
#endif

#endif
