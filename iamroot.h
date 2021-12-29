/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _IAMROOT_H_
#define _IAMROOT_H_

#ifdef __cplusplus
extern "C" {
#endif

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

int _snprintf(char *buf, size_t bufsize, const char *fmt, ...) __attribute__((format(printf,3,4)));

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
#elif defined(__x86_64__)
	return "x86_64";
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

char *sanitize(char *, size_t);
char *path_resolution(const char *, char *, size_t, int);
char *fpath_resolutionat(int, const char *, char *, size_t, int);

void __procfdname(char *, unsigned);
ssize_t __procfdreadlink(int, char *, size_t);

char *__getroot();
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
#else
#define __debug(fmt, ...)
#define __info(fmt, ...)
#define __notice(fmt, ...)
#define __warning(fmt, ...)
#endif

#define __fwarn_and_set_user_modeat(fd, path, mode, flags, user_mode) \
	({ if ((mode & user_mode) != user_mode) { \
	     __info("%s: %d/%s: Insuffisant user mode 0%03o!\n", __func__, fd, path, mode); \
	     mode |= user_mode; \
	   } })

#define __fwarn_if_insuffisant_user_modeat(fd, path, mode, flags) \
	({ if (fisdirectoryat(fd, path, flags)) { \
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
	({ if (fisdirectory(fd)) { \
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
	({ if (isdirectory(path)) { \
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

extern void __pathperror(const char *, const char *);
extern void __pathperror2(const char *, const char *, const char *);
extern void __fpathperror(int, const char *);

extern void __envperror(const char *, const char *);

extern void __pathdlperror(const char *, const char *);
#define __dlperror(s) __info("%s: %s\n", s, dlerror())

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
