/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _IAMROOT_H_
#define _IAMROOT_H_

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

#ifndef _STAT_VER
#if defined(__arm__) || defined(__mips__) || defined(__i386__)
#define _STAT_VER 3
#else
#define _STAT_VER 0
#endif
#endif

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

#define IAMROOT_XATTRS_PREFIX "user.iamroot."
#define IAMROOT_XATTRS_MODE IAMROOT_XATTRS_PREFIX "mode"
#define IAMROOT_XATTRS_UID  IAMROOT_XATTRS_PREFIX "uid"
#define IAMROOT_XATTRS_GID  IAMROOT_XATTRS_PREFIX "gid"
#endif

#if defined __FreeBSD__ || __NetBSD__
#define IAMROOT_EXTATTR_PREFIX "iamroot."
#define IAMROOT_EXTATTR_MODE IAMROOT_EXTATTR_PREFIX "mode"
#define IAMROOT_EXTATTR_UID  IAMROOT_EXTATTR_PREFIX "uid"
#define IAMROOT_EXTATTR_GID  IAMROOT_EXTATTR_PREFIX "gid"
#endif

char *_getenv(const char *);
int _clearenv();
int _putenv(char *);
int _setenv(const char *, const char *, int);
int _unsetenv(const char *);
int _snprintf(char *, size_t, const char *, ...) __attribute__((format(printf,3,4)));

int __fissymlinkat(int, const char *, int);
int __fissymlink(int);
int __issymlink(const char *);
int __fisdirectoryat(int, const char *, int);
int __fisdirectory(int);
int __isdirectory(const char *);
int __fisfileat(int, const char *, int);
int __fisfile(int);
int __isfile(const char *);
const char *__basename(const char *);
int __path_setenv(const char *, const char *, const char *, int);

int __execve(const char *, char * const [], char * const []);
int __exec_ignored(const char *);
int __issuid(const char *);
int __ldso(const char *, char * const [], char *, size_t, char *[]);
int __exec_sh(const char *, char * const *, char *[], char *, size_t);
char **__glibc_workaround(char *, size_t, char *, char *[7+1]);
ssize_t __interpreter_script_hashbang(const char *, char *, size_t);
int __interpreter_script(const char *, char * const [], char *, size_t,
			 char *[]);

char *__path_sanitize(char *, size_t);
char *__fpath(int);
char *__fpath2(int);
ssize_t fpath(int, char *, size_t);
int __path_ignored(int, const char *);
ssize_t path_resolution(int, const char *, char *, size_t, int);
char *__getpath(int, const char *, int);
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
int __setfd(int, const char *);
int __execfd();
const char *__execfn();
int __setrootdir(const char *);
const char *__getrootdir();
int __chrootdir(const char *);
int __inchroot();
char *__striprootdir(char *);

int __getno_color();
int __getcolor();

int __getfatal();

#define DEBUG_FILENO __getdebug_fd()

int __getdebug();
int __getdebug_fd();
int __verbosef(int, const char *, const char *, ...) __attribute__((format(printf,3,4)));

#if !defined(NVERBOSE)
#define __debug(fmt, ...) __verbosef(3, __func__, fmt, __VA_ARGS__)
#define __info(fmt, ...) __verbosef(2, __func__, fmt, __VA_ARGS__)
#define __notice(fmt, ...) __verbosef(1, __func__, fmt, __VA_ARGS__)
#define __warning(fmt, ...) __verbosef(0, __func__, fmt, __VA_ARGS__)
void __verbose_exec(const char *, char * const[], char * const[]);
#else
#define __debug(fmt, ...) {}
#define __info(fmt, ...) {}
#define __notice(fmt, ...) {}
#define __warning(fmt, ...) {}
#define __verbose_exec(fmt, ...) {}
#endif

#define __warn_if_not_preloading_libiamroot() \
	({ const int errno_save = errno; \
	   if (!__is_preloading_libiamroot()) \
	     __warning("%s: not preloading library!\n", __func__); \
	   errno = errno_save; \
	   })

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

#define __fwarn_if_insuffisant_user_mode(fd, mode) \
	({ if (__fisdirectory((fd)) > 0) { \
	     __fwarn_and_set_user_mode((fd), (mode), 0700); \
	   } else { \
	     __fwarn_and_set_user_mode((fd), (mode), 0600); \
	   } })

#define __fwarn_if_insuffisant_user_modeat(fd, path, mode, flags) \
	({ if (__fisdirectoryat((fd), (path), (flags)) > 0) { \
	     __fwarn_and_set_user_modeat((fd), (path), (mode), (flags), 0700); \
	   } else { \
	     __fwarn_and_set_user_modeat((fd), (path), (mode), (flags), 0600); \
	   } })

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

#define __warn_if_insuffisant_user_mode(path, mode) \
	({ if (__isdirectory((path)) > 0) { \
	     __warn_and_set_user_mode((path), (mode), 0700); \
	   } else { \
	     __warn_and_set_user_mode((path), (mode), 0600); \
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
	   } })

#define __fst_uid(fd, statbuf) \
	({ uid_t u = __fget_uid((fd)); \
	   if (u != (uid_t)-1) { \
	     statbuf->st_uid = u; \
	   } })

#define __stx_uid(path, statxbuf) \
	({ uid_t u = __get_uid((path)); \
	   if (u != (uid_t)-1) { \
	     statxbuf->stx_uid = u; \
	   } })

#define __st_gid(path, statbuf) \
	({ gid_t g = __get_gid((path)); \
	   if (g != (gid_t)-1) { \
	     statbuf->st_gid = g; \
	   } })

#define __fst_gid(fd, statbuf) \
	({ gid_t g = __fget_gid((fd)); \
	   if (g != (gid_t)-1) { \
	     statbuf->st_gid = g; \
	   } })

#define __stx_gid(path, statxbuf) \
	({ gid_t g = __get_gid((path)); \
	   if (g != (gid_t)-1) { \
	     statxbuf->stx_gid = g; \
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

#define __ignored_errno(e) (((e) == EPERM) || \
			    ((e) == EACCES) || \
			    ((e) == ENOSYS))

#define __ignored_error(rc) ((rc == -1) && (errno == EPERM))

#if defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__
#define __ignore_error_and_warn(rc, fd, path, flags) \
	({ if (__ignored_error(rc) && (__path_ignored((fd), (path)) > 0)) { \
	     __warning("%s: %s: Ignoring error '%i'!\n", __func__, __getpath((fd), (path), (flags)), rc); \
	     rc = __set_errno(0, 0); \
	   } })
#else
#define __ignore_error_and_warn(rc, fd, path, flags) \
	({ if (__ignored_error(rc) && (__path_ignored((fd), (path)) > 0)) { \
	     __warning("%s: %s: Ignoring error '%m'!\n", __func__, __getpath((fd), (path), (flags))); \
	     rc = __set_errno(0, 0); \
	   } })
#endif

#define __warn_or_fatal(fmt, ...) \
	({ __warning(fmt, __VA_ARGS__); \
	   if (__getfatal()) \
	     __abort(); })

#define __note_or_fatal(fmt, ...) \
	({ __notice(fmt, __VA_ARGS__); \
	   if (__getfatal()) \
	     __abort(); })

extern void __abort();

extern void __pathdlperror(const char *, const char *);
#define __dlperror(s) __info("%s: %s\n", s, dlerror())

#define __dl_set_errno_and_perror(e, r) \
	({ __dlperror(__func__); \
	   __set_errno(e, (r)); })

int __dlopen_needed(const char *, int);

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

int __is_preloading_libiamroot();

int __is_ldso(const char *);

ssize_t __dl_access(const char *, int, char *, size_t);

ssize_t __ldso_cache(const char *, char *, size_t);

#if defined(__GLIBC__) && (defined(__aarch64__) || defined(__x86_64__))
#define _PATH_DEFLIB "/lib64:/usr/lib64"
#elif defined(__GLIBC__)
#define _PATH_DEFLIB "/lib:/usr/lib"
#else
#define _PATH_DEFLIB "/lib:/usr/local/lib:/usr/lib"
#endif

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
