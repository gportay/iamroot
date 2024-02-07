/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>

#include "iamroot.h"

extern ssize_t next_readlinkat(int, const char *, char *, size_t);
extern int next_scandir(const char *, struct dirent ***,
			int (*)(const struct dirent *),
			int (*)(const struct dirent **,
			const struct dirent **));

#ifdef __OpenBSD__
/*
 * Stolen from musl (src/string/strchrnul.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdint.h>

#ifdef ALIGN
#undef ALIGN
#endif
#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

__attribute__((visibility("hidden")))
char *__strchrnul(const char *s, int c)
{
	c = (unsigned char)c;
	if (!c) return (char *)s + strlen(s);

#ifdef __GNUC__
	typedef size_t __attribute__((__may_alias__)) word;
	const word *w;
	for (; (uintptr_t)s % ALIGN; s++)
		if (!*s || *(unsigned char *)s == c) return (char *)s;
	size_t k = ONES * c;
	for (w = (void *)s; !HASZERO(*w) && !HASZERO(*w^k); w++);
	s = (void *)w;
#endif
	for (; *s && *(unsigned char *)s != c; s++);
	return (char *)s;
}
#endif

#ifdef __OpenBSD__
static ssize_t __fgetpath(int fd, char *buf, size_t bufsiz)
{
	const char *path;

	path = __getfd(fd);
	if (!path)
		return __set_errno(ENOENT, -1);

	_strncpy(buf, path, bufsiz);
	return strnlen(buf, bufsiz);
}
#endif

#ifdef __FreeBSD__
/*
 * Stolen from FreeBSD (lib/libutil/kinfo_getfile.c)
 *
 * SPDX-FileContributor: The Regents of the University of California
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>

__attribute__((visibility("hidden")))
struct kinfo_file *kinfo_getfile(pid_t pid, int *cntp)
{
	int mib[4];
	int error;
	int cnt;
	size_t len;
	char *buf, *bp, *eb;
	struct kinfo_file *kif, *kp, *kf;

	*cntp = 0;
	len = 0;
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_FILEDESC;
	mib[3] = pid;

	error = sysctl(mib, nitems(mib), NULL, &len, NULL, 0);
	if (error)
		return (NULL);
	len = len * 4 / 3;
	buf = malloc(len);
	if (buf == NULL)
		return (NULL);
	error = sysctl(mib, nitems(mib), buf, &len, NULL, 0);
	if (error) {
		free(buf);
		return (NULL);
	}
	/* Pass 1: count items */
	cnt = 0;
	bp = buf;
	eb = buf + len;
	while (bp < eb) {
		kf = (struct kinfo_file *)(uintptr_t)bp;
		if (kf->kf_structsize == 0)
			break;
		bp += kf->kf_structsize;
		cnt++;
	}

	kif = calloc(cnt, sizeof(*kif));
	if (kif == NULL) {
		free(buf);
		return (NULL);
	}
	bp = buf;
	eb = buf + len;
	kp = kif;
	/* Pass 2: unpack */
	while (bp < eb) {
		kf = (struct kinfo_file *)(uintptr_t)bp;
		if (kf->kf_structsize == 0)
			break;
		/* Copy/expand into pre-zeroed buffer */
		memcpy(kp, kf, kf->kf_structsize);
		/* Advance to next packed record */
		bp += kf->kf_structsize;
		/* Set field size to fixed length, advance */
		kp->kf_structsize = sizeof(*kp);
		kp++;
	}
	free(buf);
	*cntp = cnt;
	return (kif);	/* Caller must free() return value */
}

static ssize_t __fgetpath(int fd, char *buf, size_t bufsiz)
{
	struct kinfo_file *kif;
	ssize_t ret = -1;
	int i, n;

	kif = kinfo_getfile(getpid(), &n);
	if (!kif)
		return -1;

	for (i = 0; i < n; i++) {
		if (kif[i].kf_fd != fd)
			continue;

		ret = strlcpy(buf, kif[i].kf_path, bufsiz);
		break;
	}

	free(kif);
	return ret;
}
#endif

__attribute__((visibility("hidden")))
int __strtofd(const char *nptr, char **endptr)
{
	int errno_save;
	long l;

	errno_save = errno;
	errno = 0;
	l = strtol(nptr, endptr, 0);
	if (errno != 0)
		return -1;

	errno = errno_save;
	return l;
}

#if defined(__NetBSD__) || defined(__linux__)
/*
 * Stolen and hacked from musl (src/internal/procfdname.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
__attribute__((visibility("hidden")))
void __procfdname(char *buf, unsigned fd)
{
	unsigned i, j;
	for (i=0; (buf[i] = "/proc/self/fd/"[i]); i++);
	if (!fd) {
		buf[i] = '0';
		buf[i+1] = 0;
		return;
	}
	for (j=fd; j; j/=10, i++);
	buf[i] = 0;
	for (; fd; fd/=10) buf[--i] = '0' + fd%10;
}

static ssize_t __fgetpath(int fd, char *buf, size_t bufsiz)
{
	char tmp[sizeof("/proc/self/fd/") + 3*sizeof(int) + 2]; /* sign + NULL-terminated */
	const int errno_save = errno;
	ssize_t ret;
	if (fd == AT_FDCWD) {
		buf[0] = '.';
		buf[1] = 0;
		return 1;
	}
	__procfdname(tmp, fd);
	ret = next_readlinkat(AT_FDCWD, tmp, buf, bufsiz);
#ifdef __NetBSD__
	if (ret == -1 && errno == EINVAL) {
		const char *path = __getfd(fd);
		if (!path)
			return __set_errno(ENOENT, -1);
		_strncpy(buf, path, bufsiz);
		ret = strnlen(buf, bufsiz);
		return __set_errno(errno_save, ret);
	}
#endif
	if (ret == -1)
		return ret;
	return __set_errno(errno_save, ret);
}
#endif

__attribute__((visibility("hidden")))
ssize_t fpath(int fd, char *buf, size_t bufsiz)
{
	ssize_t ret;

	if (fd < 0 && fd != AT_FDCWD)
		return __set_errno(EINVAL, -1);

	ret = __fgetpath(fd, buf, bufsiz);
	if (ret == -1)
		return -1;
	buf[ret] = 0; /* ensure NULL-terminated */

	return ret;
}

__attribute__((visibility("hidden")))
char *__fpath(int fd)
{
	const int errno_save = errno;
	static char buf[PATH_MAX];
	ssize_t siz;

	*buf = 0;
	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return __set_errno(errno_save, NULL);

	return buf;
}

__attribute__((visibility("hidden")))
char *__fpath2(int fd)
{
	const int errno_save = errno;
	static char buf[PATH_MAX];
	ssize_t siz;

	*buf = 0;
	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return __set_errno(errno_save, NULL);

	return buf;
}

/*
 * Stolen and hacked from musl (src/process/execvp.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
__attribute__((visibility("hidden")))
ssize_t __path_access(const char *file, int mode, const char *path, char *buf,
		      size_t bufsiz)
{
	const char *p, *z;
	size_t l, k;
	int seen_eacces = 0;

	errno = ENOENT;
	if (!*file) return -1;

	if (!path) return -1;
	k = strnlen(file, NAME_MAX+1);
	if (k > NAME_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}
	l = strnlen(path, PATH_MAX-1)+1;

	for(p=path; ; p=z) {
		char b[l+k+1];
		z = __strchrnul(p, ':');
		if ((size_t)(z-p) >= l) {
			if (!*z++) break;
			continue;
		}
		memcpy(b, p, z-p);
		b[z-p] = '/';
		memcpy(b+(z-p)+(z>p), file, k+1);

		if (access(b, mode) != -1) {
			errno = 0;
			return path_resolution(AT_FDCWD, b, buf, bufsiz, 0);
		}
		switch (errno) {
		case EACCES:
			seen_eacces = 1;
		case ENOENT:
		case ENOTDIR:
			break;
		default:
			return -1;
		}
		if (!*z++) break;
	}
	if (seen_eacces) errno = EACCES;
	return -1;
}

__attribute__((visibility("hidden")))
int __dir_iterate(const char *path,
		  int (*callback)(const char *, const char *, void *),
		  void *user)
{
	struct dirent **namelist;
	int n, ret = 0;

	n = next_scandir(path, &namelist, NULL, alphasort);
	if (n == -1)
		return -1;

	while (n-- > 0) {
		if (strcmp(namelist[n]->d_name, ".") != 0 &&
		    strcmp(namelist[n]->d_name, "..") != 0 ) {
			if (callback(path, namelist[n]->d_name, user) == 0)
				ret++;
		}
		free(namelist[n]);
	}
	free(namelist);

	return ret;
}

__attribute__((visibility("hidden")))
const char *__getfd(int fd)
{
#if defined __OpenBSD__ || defined __NetBSD__
	char buf[NAME_MAX];
	char *ret;
	int n;

	n = _snprintf(buf, sizeof(buf), "IAMROOT_FD_%i", fd);
	if (n == -1)
		return NULL;

	ret = _getenv(buf);
	if (!ret)
		return __set_errno(ENOENT, NULL);

	__info("%i: %s='%s'\n", fd, buf, ret);

	return ret;
#else
	(void)fd;

	return NULL;
#endif
}

__attribute__((visibility("hidden")))
int __setfd(int fd, const char *path)
{
#if defined __OpenBSD__ || defined __NetBSD__
	char buf[NAME_MAX];
	int n;

	n = _snprintf(buf, sizeof(buf), "IAMROOT_FD_%i", fd);
	if (n == -1)
		return -1;

	__info("%i: %s='%s' -> '%s'\n", fd, buf, _getenv(buf), path);

	if (!path)
		return _unsetenv(buf);

	return _setenv(buf, path, 1);
#else
	(void)fd;
	(void)path;

	return 0;
#endif
}

__attribute__((visibility("hidden")))
int __execfd()
{
#if defined __OpenBSD__ || defined __NetBSD__
	char * const *p;

	for (p = environ; *p; p++) {
		char path[PATH_MAX];
		int n, err, fd;

		n = sscanf(*p, "IAMROOT_FD_%i=%" __xstr(PATH_MAX) "s", &fd, path);
		if (n != 2)
			continue;

		err = __setfd(fd, NULL);
		if (err == -1)
			__warning("%i: cannot unset fd!\n", fd);
	}
#endif

	return 0;
}

