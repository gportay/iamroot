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

int _snprintf(char *buf, size_t bufsize, const char *fmt, ...) __attribute__((format(printf,3,4)));

static inline const char *__libc()
{
#ifdef __GLIBC__
	return "glibc";
#else
	return "libc";
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
int pathsetenv(const char *, const char *, const char *, int);

char *sanitize(char *, size_t);
char *path_resolution(const char *, char *, size_t, int);
char *fpath_resolutionat(int, const char *, char *, size_t, int);

void __procfdname(char *, unsigned);
ssize_t __procfdreadlink(int, char *, size_t);

const char *getrootdir();
int chrootdir(const char *);
int inchroot();

int __getfatal();

#define DEBUG_FILENO __getdebug_fd()

int __getdebug();
int __getdebug_fd();
int __verbosef(int, const char *, const char *, ...) __attribute__((format(printf,3,4)));

#if !defined(NVERBOSE)
#define __verbose3(fmt, ...) __verbosef(3, __func__, fmt, __VA_ARGS__)
#define __verbose2(fmt, ...) __verbosef(2, __func__, fmt, __VA_ARGS__)
#define __verbose(fmt, ...) __verbosef(1, __func__, fmt, __VA_ARGS__)
#define __notice(fmt, ...) __verbosef(1, __func__, fmt, __VA_ARGS__)
#define __warning(fmt, ...) __verbosef(0, __func__, fmt, __VA_ARGS__)
#else
#define __verbose3(fmt, ...)
#define __verbose2(fmt, ...)
#define __verbose(fmt, ...)
#define __warning(fmt, ...)
#endif

#define __verbose_exec(fmt, ...) __verbose2(fmt, __VA_ARGS__)
#define __verbose_func(fmt, ...) __verbose3(fmt, __VA_ARGS__)

#define __fwarn_and_set_user_modeat(fd, path, mode, flags, user_mode) \
	({ if ((mode & user_mode) != user_mode) { \
	     __verbose2("%s: %d/%s: Insuffisant user mode 0%03o!\n", __func__, fd, path, mode); \
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
	     __verbose2("%s: %d: Insuffisant user mode 0%03o!\n", __func__, fd, mode); \
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
	     __verbose2("%s: %s: Insuffisant user mode 0%03o!\n", __func__, path, mode); \
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
	     __verbose2("%s: Too restrictive umask 0%03o!\n", __func__, mask); \
	     mask &= ~(user_mask); \
	   } })

#define __warn_if_too_restrictive_umask(mask) \
	({ __warn_and_set_umask(mask, 0400); })

extern void __perror(const char *, const char *);
extern void __perror2(const char *, const char *, const char *);
extern void __fperror(int, const char *);

#define __dl_perror(s) __verbose2("%s: %s\n", s, dlerror())

#ifdef __cplusplus
}
#endif

#endif
