/*
 * Copyright 2021-2023 Gaël PORTAY
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
#define __streq(s1,s2) (strcmp((s1),(s2)) == 0)

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
#define __strlen(s) strnlen((s), PATH_MAX)
#define __strnlen(s) strnlen((s), sizeof((s))-1)
#define __strlcmp(s1, s2) strncmp((s1), (s2), __strlen((s2)))
#define __strncmp(s1, s2) strncmp((s1), (s2), sizeof((s2))-1)
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

/* See https://www.in-ulm.de/~mascheck/various/shebang/#results */
#define HASHBANG_MAX NAME_MAX

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

#ifdef __FreeBSD__
#define IAMROOT_EXTATTR_PREFIX "iamroot."
#define IAMROOT_EXTATTR_MODE IAMROOT_EXTATTR_PREFIX "mode"
#define IAMROOT_EXTATTR_UID  IAMROOT_EXTATTR_PREFIX "uid"
#define IAMROOT_EXTATTR_GID  IAMROOT_EXTATTR_PREFIX "gid"
#endif

int _snprintf(char *, size_t, const char *, ...) __attribute__((format(printf,3,4)));

static inline const char *__libc()
{
#if defined(__GLIBC__)
	return "glibc";
#elif defined(__FreeBSD__)
	return "FreeBSD libc";
#else
	return "libc";
#endif
}

static inline const char *__arch()
{
#if defined(__aarch64__)
	return "aarch64";
#elif defined(__arm__)
	return "arm";
#elif defined(__x86_64__)
	return "x86_64";
#elif defined(__i386__)
	return "i686";
#else
	return "unknown";
#endif
}

int fissymlinkat(int, const char *, int);
int fissymlink(int);
int issymlink(const char *);
int fisdirectoryat(int, const char *, int);
int fisdirectory(int);
int isdirectory(const char *);
int fisfileat(int, const char *, int);
int fisfile(int);
int isfile(const char *);
int pathprependenv(const char *, const char *, int);
int pathsetenv(const char *, const char *, const char *, int);

int __execve(const char *, char * const [], char * const []);
int exec_ignored(const char *);
int __ld_linux_has_inhibit_cache_option(const char *);
int __ld_linux_has_argv0_option(const char *);
int __ld_linux_has_preload_option(const char *);
int issuid(const char *);
ssize_t getinterp(const char *, char *, size_t);
ssize_t gethashbang(const char *, char *, size_t);
char *__ld_preload(const char *, int);
char *__ld_library_path(const char *, int);
char *__rpath(const char *);
char *__runpath(const char *);
char *__inhibit_rpath();
char *__getexec();
int __loader(const char *, char * const [], char *, size_t, char *[]);
int __exec_sh(const char *, char * const *, char **, char *, size_t);
int __hashbang(const char *, char * const [], char *, size_t, char *[]);

char *__basename(char *);
char *path_sanitize(char *, size_t);
ssize_t fpath(int, char *, size_t);
int path_ignored(int, const char *);
ssize_t path_resolution(int, const char *, char *, size_t, int);
char *__getpath(int, const char *, int);
ssize_t path_access(const char *, int, const char *, char *, size_t);
int path_iterate(const char *, int (*)(const char *, void *), void *);

void __procfdname(char *, unsigned);
ssize_t __procfdreadlink(int, char *, size_t);

char *__getroot();
const char *__getexe();

const char *getrootdir();
int chrootdir(const char *);
int inchroot();
char *striprootdir(char *);

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
void verbose_exec(const char *, char * const[], char * const[]);
#else
#define __debug(fmt, ...)
#define __info(fmt, ...)
#define __notice(fmt, ...)
#define __warning(fmt, ...)
#define verbose_exec(fmt, ...)
#endif

#define __remove_at_empty_path_if_needed(path, flags) \
	({ if ((*path) && (flags & AT_EMPTY_PATH)) { \
	     flags &= ~AT_EMPTY_PATH; \
	   } })

#define __fwarn_and_set_user_modeat(fd, path, mode, flags, user_mode) \
	({ if ((mode & user_mode) != user_mode) { \
	     __info("%s: %i/%s: Insuffisant user mode 0%03o!\n", __func__, fd, path, mode); \
	     mode |= user_mode; \
	   } \
	   if (mode & S_ISUID) { \
	     __info("%s: %i/%s: SUID bit 0%04o!\n", __func__, fd, path, mode); \
	     mode &= ~S_ISUID; \
	   } \
	   if (mode & S_ISGID) { \
	     __info("%s: %i/%s: SGID bit 0%04o!\n", __func__, fd, path, mode); \
	     mode &= ~S_ISGID; \
	   } })

#define __fwarn_if_insuffisant_user_modeat(fd, path, mode, flags) \
	({ if (fisdirectoryat(fd, path, flags) > 0) { \
	     __fwarn_and_set_user_modeat(fd, path, mode, flags, 0700); \
	   } else { \
	     __fwarn_and_set_user_modeat(fd, path, mode, flags, 0600); \
	   } })

#define __warn_and_set_user_mode(path, mode, user_mode) \
	({ if ((mode & user_mode) != user_mode) { \
	     __info("%s: %s: Insuffisant user mode 0%03o!\n", __func__, path, mode); \
	     mode |= user_mode; \
	   } \
	   if (mode & S_ISUID) { \
	     __info("%s: %s: SUID bit 0%04o!\n", __func__, path, mode); \
	     mode &= ~S_ISUID; \
	   } \
	   if (mode & S_ISGID) { \
	     __info("%s: %s: SGID bit 0%04o!\n", __func__, path, mode); \
	     mode &= ~S_ISGID; \
	   } })

#define __warn_if_insuffisant_user_mode(path, mode) \
	({ if (isdirectory(path) > 0) { \
	     __warn_and_set_user_mode(path, mode, 0700); \
	   } else { \
	     __warn_and_set_user_mode(path, mode, 0600); \
	   } })

#define __warn_and_set_umask(mask, user_mask) \
	({ if (mask & user_mask) { \
	     __info("%s: Too restrictive umask 0%03o!\n", __func__, mask); \
	     mask &= ~(user_mask); \
	   } })

#define __warn_if_too_restrictive_umask(mask) \
	({ __warn_and_set_umask(mask, 0400); })

#ifdef __linux__
extern ssize_t next_lgetxattr(const char *, const char *, void *, size_t);
extern int next_lsetxattr(const char *, const char *, const void *, size_t,
			  int);
extern int next_lremovexattr(const char *, const char *);

#define __get_mode(path) \
	({ int save_errno = errno; \
	   mode_t m; \
	   if (next_lgetxattr(buf, IAMROOT_XATTRS_MODE, &m, sizeof(m)) != sizeof(m)) \
		m = (mode_t)-1; \
	   errno = save_errno; \
	   m; \
	})

#define __set_mode(path, oldmode, mode) \
	({ int save_errno = errno; \
	   if ((oldmode) == (mode)) { \
	     next_lremovexattr((path), IAMROOT_XATTRS_MODE); \
	   } else { \
	     next_lsetxattr((path), IAMROOT_XATTRS_MODE, &(oldmode), \
			    sizeof((oldmode)), 0); \
	   } \
	   errno = save_errno; \
	   0; \
	})

#define __get_uid(path) \
	({ int save_errno = errno; \
	   uid_t u; \
	   if (next_lgetxattr(buf, IAMROOT_XATTRS_UID, &u, sizeof(u)) != sizeof(u)) \
		u = (uid_t)-1; \
	   errno = save_errno; \
	   u; \
	})

#define __set_uid(path, olduid, uid) \
	({ int save_errno = errno; \
	   if ((olduid) == (uid)) { \
	     next_lremovexattr((path), IAMROOT_XATTRS_UID); \
	   } else { \
	     next_lsetxattr((path), IAMROOT_XATTRS_UID, &(olduid), \
			    sizeof((olduid)), 0); \
	   } \
	   errno = save_errno; \
	   0; \
	})

#define __get_gid(path) \
	({ int save_errno = errno; \
	   gid_t g; \
	   if (next_lgetxattr(buf, IAMROOT_XATTRS_GID, &g, sizeof(g)) != sizeof(g)) \
		g = (gid_t)-1; \
	   errno = save_errno; \
	   g; \
	})

#define __set_gid(path, oldgid, gid) \
	({ int save_errno = errno; \
	   if ((oldgid) == (gid)) { \
	     next_lremovexattr((path), IAMROOT_XATTRS_GID); \
	   } else { \
	     next_lsetxattr((path), IAMROOT_XATTRS_GID, &(oldgid), \
			    sizeof((oldgid)), 0); \
	   } \
	   errno = save_errno; \
	   0; \
	})
#endif

#ifdef __FreeBSD__
extern ssize_t next_extattr_get_link(const char *, int, const char *, void *,
				     size_t);
extern ssize_t next_extattr_set_link(const char *, int, const char *, const void *,
				 size_t);
extern int next_extattr_delete_link(const char *, int, const char *);

#define __get_mode(path) \
	({ int save_errno = errno; \
	   mode_t m; \
	   if (next_extattr_get_link(buf, EXTATTR_NAMESPACE_USER, \
				     IAMROOT_EXTATTR_MODE, &m, sizeof(m)) \
				     != sizeof(m)) \
		m = (mode_t)-1; \
	   errno = save_errno; \
	   m; \
	})

#define __set_mode(path, oldmode, mode) \
	({ int save_errno = errno; \
	   if ((oldmode) == (mode)) { \
	     next_extattr_delete_link((path), EXTATTR_NAMESPACE_USER, \
				      IAMROOT_EXTATTR_MODE); \
	   } else { \
	     next_extattr_set_link((path), EXTATTR_NAMESPACE_USER, \
				   IAMROOT_EXTATTR_MODE, &(oldmode), \
				   sizeof((oldmode))); \
	   } \
	   errno = save_errno; \
	   0; \
	})

#define __get_uid(path) \
	({ int save_errno = errno; \
	   uid_t u; \
	   if (next_extattr_get_link(buf, EXTATTR_NAMESPACE_USER, \
				     IAMROOT_EXTATTR_UID, &u, sizeof(u)) \
				     != sizeof(u)) \
		u = (uid_t)-1; \
	   errno = save_errno; \
	   u; \
	})

#define __set_uid(path, olduid, uid) \
	({ int save_errno = errno; \
	   if ((olduid) == (uid)) { \
	     next_extattr_delete_link((path), EXTATTR_NAMESPACE_USER, \
				      IAMROOT_EXTATTR_UID); \
	   } else { \
	     next_extattr_set_link((path), EXTATTR_NAMESPACE_USER, \
				   IAMROOT_EXTATTR_UID, &(olduid), \
				   sizeof((olduid))); \
	   } \
	   errno = save_errno; \
	   0; \
	})

#define __get_gid(path) \
	({ int save_errno = errno; \
	   gid_t g; \
	   if (next_extattr_get_link(buf, EXTATTR_NAMESPACE_USER, \
				     IAMROOT_EXTATTR_MODE, &g, sizeof(g)) \
				     != sizeof(g)) \
		g = (gid_t)-1; \
	   errno = save_errno; \
	   g; \
	})

#define __set_gid(path, oldgid, gid) \
	({ int save_errno = errno; \
	   if ((oldgid) == (gid)) { \
	     next_extattr_delete_link((path), EXTATTR_NAMESPACE_USER, \
				      IAMROOT_EXTATTR_GID); \
	   } else { \
	     next_extattr_set_link((path), EXTATTR_NAMESPACE_USER, \
				   IAMROOT_EXTATTR_GID, &(oldgid), \
				   sizeof((oldgid))); \
	   } \
	   errno = save_errno; \
	   0; \
	})
#endif

#define __st_mode(path, statbuf) \
	({ mode_t m = __get_mode(path); \
	   if (m != (mode_t)-1) { \
	     statbuf->st_mode = (statbuf->st_mode & S_IFMT) | m; \
	   } })

#define __stx_mode(path, statxbuf) \
	({ mode_t m = __get_mode(path); \
	   if (m != (mode_t)-1) { \
	     statxbuf->stx_mode = (statxbuf->stx_mode & S_IFMT) | m; \
	   } })

#define __st_uid(path, statbuf) \
	({ uid_t u = __get_uid(path); \
	   if (u != (uid_t)-1) { \
	     statbuf->st_uid = u; \
	   } })

#define __stx_uid(path, statxbuf) \
	({ uid_t u = __get_uid(path); \
	   if (u != (uid_t)-1) { \
	     statxbuf->stx_uid = u; \
	   } })

#define __st_gid(path, statbuf) \
	({ gid_t g = __get_gid(path); \
	   if (g != (gid_t)-1) { \
	     statbuf->st_gid = g; \
	   } })

#define __stx_gid(path, statxbuf) \
	({ gid_t g = __get_gid(path); \
	   if (g != (gid_t)-1) { \
	     statxbuf->stx_gid = g; \
	   } })

#define __ignored_errno(e) (((e) != EPERM) && ((e) != EACCES) && ((e) != ENOSYS))

#define __ignored_error(rc) ((rc == -1) && (errno == EPERM))

#define __ignore_error_and_warn(rc, fd, path, flags) \
	({ if (__ignored_error(rc) && (path_ignored(fd, path) > 0)) { \
	     __warning("%s: %s: Ignoring error '%m'!\n", __func__, __getpath(fd, path, flags)); \
	     rc = 0; \
	     errno = 0; \
	   } })

#define __warn_or_fatal(fmt, ...) \
	({ __warning(fmt, __VA_ARGS__); \
	   if (__getfatal()) \
	     __abort(); })

#define __note_or_fatal(fmt, ...) \
	({ __notice(fmt, __VA_ARGS__); \
	   if (__getfatal()) \
	     __abort(); })

extern void __abort();

extern void __pathperror(const char *, const char *);
extern void __pathperror2(const char *, const char *, const char *);
extern void __fpathperror(int, const char *);

extern void __envperror(const char *, const char *);

extern void __pathdlperror(const char *, const char *);
#define __dlperror(s) __info("%s: %s\n", s, dlerror())

int close(int);
static inline void __close(int fd)
{
	extern int errno;
	int save_errno;

	save_errno = errno;
	if (close(fd))
		__fpathperror(fd, "close");
	errno = save_errno;
}

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
