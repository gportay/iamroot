/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/stat.h>
#if defined __linux__ || defined __FreeBSD__
#include <sys/auxv.h>
#endif
#include <regex.h>

#include "iamroot.h"

extern char *next_getcwd(char *, size_t);
extern int next_fstat(int, struct stat *);
extern int next_fstatat(int, const char *, struct stat *, int);
extern ssize_t next_readlinkat(int, const char *, char *, size_t);
extern int next_scandir(const char *, struct dirent ***,
			int (*)(const struct dirent *),
			int (*)(const struct dirent **,
			const struct dirent **));

#ifdef __linux__
__attribute__((visibility("hidden")))
int __secure()
{
	return getauxval(AT_SECURE) != 0;
}
#endif

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

__attribute__((visibility("hidden")))
int _snprintf(char *buf, size_t bufsiz, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vsnprintf(buf, bufsiz, fmt, ap);
	va_end(ap);

	if (ret == -1)
		return -1;

	if ((size_t)ret < bufsiz)
		return ret;

	return __set_errno(ENOSPC, -1);
}

__attribute__((visibility("hidden")))
int __fissymlinkat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fissymlink(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __issymlink(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fisdirectoryat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __isdirectory(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fisdirectory(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fisfileat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fisfile(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __isfile(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
const char *__basename(const char *path)
{
	char *s = strrchr(path, '/');
	if (!s)
		return path;

	return s+1; /* trailing-slash */
}

int __path_setenv(const char *root, const char *name, const char *value,
		  int overwrite)
{
	size_t rootlen, vallen, newlen;

	if (!name || !value)
		return __set_errno(EINVAL, -1);

	if (!root)
		goto setenv;

	newlen = 0;
	rootlen = __strlen(root);

	vallen = __strlen(value);
	if (vallen > 0) {
		char val[vallen+1]; /* NULL-terminated */
		char *token, *saveptr;

		newlen = vallen;
		newlen += rootlen;
		newlen++; /* ensure NULL-terminated */

		__strncpy(val, value);
		token = strtok_r(val, ":", &saveptr);
		if (token && *token)
			while (strtok_r(NULL, ":", &saveptr))
				newlen += rootlen;
	}

	if (newlen > 0) {
		char val[vallen+1], new_value[newlen+1]; /* NULL-terminated */
		char *str, *token, *saveptr;

		str = new_value;
		__strncpy(val, value);
		token = strtok_r(val, ":", &saveptr);
		if (token && *token) {
			int n;

			n = _snprintf(str, newlen, "%s%s", root, token);
			if ((n == -1) || (newlen < (size_t)n))
				return __set_errno(EOVERFLOW, -1);
			str += n;
			newlen -= n;
			while ((token = strtok_r(NULL, ":", &saveptr))) {
				n = _snprintf(str, newlen, ":%s%s", root, token);
				if ((n == -1) || (newlen < (size_t)n)) {
					errno = EOVERFLOW;
					return -1;
				}
				str += n;
				newlen -= n;
			}
		}

		return _setenv(name, new_value, overwrite);
	}

setenv:
	return _setenv(name, value, overwrite);
}

__attribute__((visibility("hidden")))
int __execve(const char *path, char * const argv[], char * const envp[])
{
	const char *root;
	ssize_t len;

	root = __getrootdir();
	if (streq(root, "/"))
		goto exit;

	len = __strlen(root);
	if (strneq(path, root, len))
		path += len;

exit:
	return execve(path, argv, envp);
}

__attribute__((visibility("hidden")))
int __issuid(const char *path)
{
	struct stat statbuf;
	int ret = -1;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, 0);
	if (ret == -1)
		return -1;

	return (statbuf.st_mode & S_ISUID) != 0;
}

static char *__getexec()
{
	char *ret;

	ret = _getenv("IAMROOT_EXEC");
	if (!ret)
		return __xstr(PREFIX)"/lib/iamroot/exec.sh";

	return ret;
}

__attribute__((visibility("hidden")))
int __exec_sh(const char *path, char * const *argv, char *interparg[],
	      char *buf, size_t bufsiz)
{
	int i, err;

	/*
	 * Run exec.sh from the host system.
	 *
	 * The library and the script exec.sh are host system binaries, it must
	 * be run from the host system. Furthermore, the intepreter for the
	 * script exec.sh is not necessarily part of the chroot environment.
	 */
	i = 0;
	interparg[i++] = _strncpy(buf, __getexec(), bufsiz);
	interparg[i++] = (char *)path; /* original path as first positional
					* argument
					*/
	interparg[i] = NULL; /* ensure NULL-terminated */

	/* Set library version for future API compatibility. */
	err = _setenv("IAMROOT_VERSION", __xstr(VERSION), 1);
	if (err)
		return -1;

	/* Set argv0 needed for binaries such as kmod, busybox... */
	err = _setenv("_argv0", *argv, 1);
	if (err)
		return -1;

	/*
	 * Unset the dynamic loader environment variables LD_PRELOAD and
	 * LD_LIBRARY_PATH that referts to libraries and paths in the chroot
	 * environment.
	 *
	 * And back them up if needed.
	 */
	err = _setenv("_preload", _getenv("LD_PRELOAD") ?: "", 1);
	if (err)
		return -1;

	err = _setenv("_library_path", _getenv("LD_LIBRARY_PATH") ?: "", 1);
	if (err)
		return -1;

	err = _unsetenv("LD_PRELOAD");
	if (err)
		return -1;

	err = _unsetenv("LD_LIBRARY_PATH");
	if (err)
		return -1;

	return i;
}

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

static int __strtok(const char *str, const char *delim,
		    int (*callback)(const char *, void *), void *user)
{
	size_t len;

	if (!str || !callback)
		return __set_errno(EINVAL, -1);

	len = __strlen(str);
	if (len > 0) {
		char buf[len+1]; /* NULL-terminated */
		char *token, *saveptr;

		__strncpy(buf, str);
		token = strtok_r(buf, delim, &saveptr);
		do {
			int ret;

			if (!token)
				break;

			ret = callback(token, user);
			if (ret != 0)
				return ret;
		} while ((token = strtok_r(NULL, delim, &saveptr)));
	}

	return 0;
}

__attribute__((visibility("hidden")))
int __path_iterate(const char *path, int (*callback)(const char *, void *),
		   void *user)
{
	return __strtok(path, ":", callback, user);
}

__attribute__((visibility("hidden")))
int __group_iterate(const char *path, int (*callback)(const char *, void *),
		    void *user)
{
	return __strtok(path, " ", callback, user);
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
char *__getroot()
{
	return _getenv("IAMROOT_ROOT");
}

__attribute__((visibility("hidden")))
const char *__getexe()
{
	const char *exec;
	size_t len;
	char *root;

	exec = __execfn();
	if (!exec)
		return NULL;

	len = 0;
	root = __getroot();
	if (root)
		len = __strlen(root);

	if (__strlen(exec) < len)
		return NULL;

	return &exec[len];
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

#ifdef __linux__
__attribute__((visibility("hidden")))
const char *__execfn()
{
	return (const char *)getauxval(AT_EXECFN);
}
#endif

#ifdef __FreeBSD__
__attribute__((visibility("hidden")))
const char *__execfn()
{
	static char buf[PATH_MAX];
	int ret;

	ret = elf_aux_info(AT_EXECPATH, buf, sizeof(buf));
	if (ret == -1)
		return NULL;

	return buf;
}
#endif

#ifdef __OpenBSD__
/*
 * Stolen and hacked from OpenBSD (usr.bin/top/machine.c)
 *
 * SPDX-FileCopyrightText: The OpenBSD Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <sys/sysctl.h>

#define err(e, r) return __set_errno((e), (r))

__attribute__((visibility("hidden")))
static char **get_proc_args()
{
	static char **s;
	static size_t siz = 1023;
	int mib[4];

	if (!s && !(s = malloc(siz)))
		err(1, NULL);

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC_ARGS;
	mib[2] = getpid();
	mib[3] = KERN_PROC_ARGV;
	for (;;) {
		size_t space = siz;
		if (sysctl(mib, 4, s, &space, NULL, 0) == 0)
			break;
		if (errno != ENOMEM)
			return NULL;
		siz *= 2;
		if ((s = realloc(s, siz)) == NULL)
			err(1, NULL);
	}
	return s;
}
#undef err

__attribute__((visibility("hidden")))
const char *__execfn()
{
	static char buf[PATH_MAX];
	char **argv;
	ssize_t siz;

	if (*buf)
		return buf;

	argv = get_proc_args();
	if (!argv)
		return NULL;

	if (*argv[0] == '/')
		siz = path_resolution(AT_FDCWD, argv[0], buf, sizeof(buf), 0);
	else
		siz = __path_access(argv[0], X_OK, _getenv("PATH"), buf,
				    sizeof(buf));
	if (siz == -1)
		return NULL;

	return buf;
}
#endif

#ifdef __NetBSD__
__attribute__((visibility("hidden")))
const char *__execfn()
{
	static char buf[PATH_MAX];
	ssize_t siz;

	if (*buf)
		return buf;

	siz = next_readlinkat(AT_FDCWD, "/proc/self/exe", buf, sizeof(buf));
	if (siz == -1)
		return NULL;
	buf[siz] = 0; /* ensure NULL-terminated */

	return buf;
}
#endif

__attribute__((visibility("hidden")))
int __setrootdir(const char *path)
{
	if (!path) {
		__info("Exiting chroot: '%s'\n", __getrootdir());
		return _unsetenv("IAMROOT_ROOT");
	}

	__info("Enterring chroot: '%s'\n", path);
	return _setenv("IAMROOT_ROOT", path, 1);
}

__attribute__((visibility("hidden")))
const char *__getrootdir()
{
	char *root;

	root = __getroot();
	if (!root)
		return "/";

	return root;
}

__attribute__((visibility("hidden")))
int __chrootdir(const char *cwd)
{
	char buf[PATH_MAX];
	const char *root;

	if (cwd == NULL)
		cwd = next_getcwd(buf, sizeof(buf));

	if (cwd == NULL)
		return -1;

	root = __getrootdir();
	if (!__strleq(cwd, root))
		return __setrootdir(NULL);

	return 0;
}

__attribute__((visibility("hidden")))
int __inchroot()
{
	return !streq(__getrootdir(), "/");
}

__attribute__((visibility("hidden")))
char *__striprootdir(char *path)
{
	const char *root;
	size_t len, size;
	char *ret;

	if (!path || !*path)
		return __set_errno(EINVAL, NULL);

	root = __getrootdir();
	if (streq(root, "/"))
		return path;

	ret = path;
	size = __strlen(ret);
	len = __strlen(root);
	if (strneq(root, ret, len))
		memmove(ret, &ret[len], size-len+1); /* NULL-terminated */

	if (!*ret)
		_strncpy(ret, "/", size);

	return ret;
}

__attribute__((visibility("hidden")))
int __getno_color()
{
	return strtol(_getenv("NO_COLOR") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getcolor()
{
	const int errno_save = errno;

	if (!isatty(__getdebug_fd()))
		return __set_errno(errno_save, 0);

	return __getno_color() == 0;
}

__attribute__((visibility("hidden")))
int __getfatal()
{
	return strtol(_getenv("IAMROOT_FATAL") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getdebug()
{
	return strtol(_getenv("IAMROOT_DEBUG") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getdebug_fd()
{
	return strtol(_getenv("IAMROOT_DEBUG_FD") ?: "2", NULL, 0);
}

typedef struct {
	regex_t re;
#ifndef JIMREGEXP_H
	char jimpad[40];
#endif
} __regex_t;

static regex_t *re_ignore;

static void __regex_perror(const char *s, regex_t *regex, int err)
{
	char buf[128];
	regerror(err, regex, buf, sizeof(buf));
	if (!s) {
		dprintf(DEBUG_FILENO, "%s\n", buf);
		return;
	}

	dprintf(DEBUG_FILENO, "%s: %s\n", s, buf);
}

__attribute__((constructor,visibility("hidden")))
void verbosef_init()
{
	static __regex_t regex_ignore;
	const char *ignore;
	int ret;

	ignore = _getenv("IAMROOT_DEBUG_IGNORE");
	if (!ignore)
		ignore = "^$";

	ret = regcomp(&regex_ignore.re, ignore, REG_NOSUB|REG_EXTENDED);
	if (ret == -1) {
		__regex_perror("regcomp", &regex_ignore.re, ret);
		return;
	}

	re_ignore = &regex_ignore.re;
}

__attribute__((destructor,visibility("hidden")))
void verbosef_fini()
{
	if (!re_ignore)
		return;

	regfree(re_ignore);
	re_ignore = NULL;
}

static int __ignore(const char *func)
{
	int ret = 0;

	if (!re_ignore)
		return 0;

	ret = regexec(re_ignore, func, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_ignore, ret);
		return 0;
	}

	return !ret;
}
static int __vdverbosef(int fd, int lvl, const char *func, const char *fmt,
			va_list ap)
{
	int debug;
	int color;
	int ret;

	if (lvl != 0 && __ignore(func))
		return 0;

	debug = __getdebug();
	if (debug < lvl)
		return 0;

	color = __getcolor();
	if (color) {
		if (lvl == 0)
			dprintf(fd, "\033[31;1m");
		else
			dprintf(fd, "\033[32;1m");
	}

	ret = dprintf(fd, "%s: ", lvl == 0 ? "Warning" : "Debug");

	if (color)
		dprintf(fd, "\033[0m");

	ret += vdprintf(fd, fmt, ap);
	return ret;
}

__attribute__((visibility("hidden")))
int __verbosef(int lvl, const char *func, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vdverbosef(DEBUG_FILENO, lvl, func, fmt, ap);
	va_end(ap);
	return ret;
}

#if !defined(NVERBOSE)
__attribute__((visibility("hidden")))
void __verbose_exec(char * const argv[], char * const envp[])
{
	int color, fd, debug;
	char * const *p;

	debug = __getdebug();
	if (debug == 0)
		return;

	fd = __getdebug_fd();
	if (fd < 0)
		return;

	color = __getcolor();
	if (color)
		dprintf(fd, "\033[32;1m");

	dprintf(fd, "Debug: ");

	if (color)
		dprintf(fd, "\033[0m");


	dprintf(fd, "running");

	for (p = envp; *p; p++) {
		if (strchr(*p, ' '))
			dprintf(fd, " \"%s\"", *p);
		else
			dprintf(fd, " %s", *p);
	}

	for (p = argv; *p; p++) {
		if (strchr(*p, ' '))
			dprintf(fd, " \"%s\"", *p);
		else
			dprintf(fd, " %s", *p);
	}

	dprintf(fd, "\n");
}
#endif

__attribute__((visibility("hidden")))
void __abort()
{
	raise(SIGABRT);
}

__attribute__((visibility("hidden")))
void __pathdlperror(const char *path, const char *s)
{
	const char *p = path && *path ? path : "(empty)";
	(void)p;
	(void)s;

	__note_or_fatal("%s: %s: %s\n", p, s, dlerror());
}
