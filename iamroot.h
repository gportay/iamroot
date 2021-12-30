/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _IAMROOT_H_
#define _IAMROOT_H_

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
	   strncpy((s1), (s2), l-1); \
	   (s1)[l] = 0; \
	   (s1); })
#define __strchrnul strchrnul
#define __strlcmp(s1, s2) strncmp(s1, s2, strlen(s2))
#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)
#define __strlcpy(s1, s2) \
	({ const int l = strlen((s2)); \
	   strncpy((s1), (s2), l); \
	   (s1)[l] = 0; \
	   (s1); })
#define __strncpy(s1, s2) \
	({ const int l = sizeof((s1))-1; \
	   strncpy((s1), (s2), l); \
	   (s1)[l] = 0; \
	   (s1); })
#define __strncat(s1, s2) \
	({ const int l = sizeof((s1))-1; \
	   strncat((s1), (s2), l); \
	   (s1)[l] = 0; \
	   (s1); })

/* See https://www.in-ulm.de/~mascheck/various/shebang/#results */
#define HASHBANG_MAX NAME_MAX

extern char **__environ;

int _snprintf(char *, size_t, const char *, ...) __attribute__((format(printf,3,4)));

static inline const char *__libc()
{
#ifdef __GLIBC__
	return "glibc";
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

int exec_ignored(const char *);
int __ld_linux_has_argv0_option(const char *);
int __ld_linux_has_preload_option(const char *);
int issuid(const char *);
ssize_t getinterp(const char *, char *, size_t);
ssize_t gethashbang(const char *, char *, size_t);
char *__ld_preload(const char *, int);
char *__ld_library_path(const char *, int, const char *, const char *);
char *__rpath(const char *);
char *__runpath(const char *);
char *__inhibit_rpath();
char *__getexec();

char *__basename(char *);
char *sanitize(char *, size_t);
int path_ignored(int, const char *);
char *path_resolution(int, const char *, char *, size_t, int);
char *__getpath(int, const char *, int);
char *path_access(const char *, int, const char *, char *, size_t);

void __procfdname(char *, unsigned);
ssize_t __procfdreadlink(int, char *, size_t);

char *__getroot();
const char *__getexe();

const char *getrootdir();
int chrootdir(const char *);
int inchroot();

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

#define __fwarn_and_set_user_modeat(fd, path, mode, flags, user_mode) \
	({ if ((mode & user_mode) != user_mode) { \
	     __info("%s: %d/%s: Insuffisant user mode 0%03o!\n", __func__, fd, path, mode); \
	     mode |= user_mode; \
	   } })

#define __fwarn_if_insuffisant_user_modeat(fd, path, mode, flags) \
	({ if (fisdirectoryat(fd, path, flags) > 0) { \
	     __fwarn_and_set_user_modeat(fd, path, mode, flags, 0700); \
	   } else { \
	     __fwarn_and_set_user_modeat(fd, path, mode, flags, 0600); \
	   } })

#define __fwarn_and_set_user_mode(fd, mode, user_mode) \
	({ if ((mode & user_mode) != user_mode) { \
	     __info("%s: %d: Insuffisant user mode 0%03o!\n", __func__, fd, mode); \
	     mode |= user_mode; \
	   } })

#define __fwarn_if_insuffisant_user_mode(fd, mode) \
	({ if (fisdirectory(fd) > 0) { \
	     __fwarn_and_set_user_mode(fd, mode, 0700); \
	   } else { \
	     __fwarn_and_set_user_mode(fd, mode, 0600); \
	   } })

#define __warn_and_set_user_mode(path, mode, user_mode) \
	({ if ((mode & user_mode) != user_mode) { \
	     __info("%s: %s: Insuffisant user mode 0%03o!\n", __func__, path, mode); \
	     mode |= user_mode; \
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

#define __ignored_error(rc) ((rc == -1) && (errno == EPERM))

#define __ignore_error_and_warn(rc, fd, path, flags) \
	({ if (__ignored_error(rc) && (path_ignored(fd, path) > 0)) { \
	     __warning("%s: %s: Ignoring error '%m'!\n", __func__, __getpath(fd, path, flags)); \
	     rc = 0; \
	     errno = 0; \
	   } })

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
