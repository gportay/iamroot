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
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <elf.h>
#include <sys/stat.h>
#if defined __linux__ || defined __FreeBSD__
#include <sys/auxv.h>
#endif
#include <pthread.h>

#include "iamroot.h"

#include "jimregexp.h"

#define SCRIPTMAG "#!"
#define SSCRIPTMAG 2

extern char *next_getcwd(char *, size_t);
extern int next_faccessat(int, const char *, int, int);
#ifdef __NetBSD__
#define next_fstat next___fstat50
extern int next___fstat50(int, struct stat *);
#else
extern int next_fstat(int, struct stat *);
#endif
extern int next_fstatat(int, const char *, struct stat *, int);
extern int next_open(const char *, int, mode_t);
extern ssize_t next_readlinkat(int, const char *, char *, size_t);
extern int next_scandir(const char *, struct dirent ***,
			int (*)(const struct dirent *),
			int (*)(const struct dirent **,
			const struct dirent **));

#ifdef __linux__
hidden int __secure()
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

hidden char *__strchrnul(const char *s, int c)
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

hidden int _snprintf(char *buf, size_t bufsiz, const char *fmt, ...)
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

hidden int __fis_symlinkat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

hidden int __fis_symlink(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

hidden int __is_symlink(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

hidden int __fis_directoryat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

hidden int __is_directory(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

hidden int __fis_directory(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

hidden int __fis_fileat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

hidden int __fis_file(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

hidden int __is_file(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

hidden const char *__basename(const char *path)
{
	char *s = strrchr(path, '/');
	if (!s)
		return path;

	return s+1; /* trailing-slash */
}

static int __fcan_exec(int fd)
{
	struct stat statbuf;
	char magic[4];
	ssize_t siz;
	int err;

	/* Get file status */
	err = fstat(fd, &statbuf);
	if (err == -1)
		return -1;

	/* File is non-executable */
	if ((statbuf.st_mode & S_IXUSR) == 0)
		return __set_errno(ENOEXEC, -1);

	/* Read magic */
	siz = read(fd, magic, sizeof(magic));
	if (siz == -1)
		return -1;
	else if (siz != 0 && (size_t)siz < sizeof(magic))
		return __set_errno_and_perror(EIO, -1);

	/* It is an empty file */
	if (siz == 0)
		return 1;

	/* It is an interpreter-script */
	if (memcmp(magic, SCRIPTMAG, SSCRIPTMAG) == 0)
		return 1;

	/* It is an ELF */
	if (memcmp(magic, ELFMAG, SELFMAG) == 0)
		return __elf_has_interp(fd);

	/* Unsupported yet! */
	return __set_errno(ENOTSUP, -1);
}

hidden int __can_exec(const char *path)
{
	int fd = -1, ret;

	/*
	 * In secure-execution mode, preload pathnames containing slashes are
	 * ignored. Furthermore, shared objects are preloaded only from the
	 * standard search directories and only if they have set-user-ID mode
	 * bit enabled (which is not typical).
	 */
	ret = __is_suid(path);
	if (ret == -1)
		return -1;
	if (ret == 1)
		__notice("%s: SUID executable\n", path);
	if (ret == 1)
		return 0;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	/*
	 * Only ELF shared object and interpreter-script are supported.
	 */
	ret = __fcan_exec(fd);
	if (ret == 0)
		__notice("%s: static-pie executable\n", path);

	__close(fd);

	return ret;
}

hidden int __execve(const char *path, char * const argv[], char * const envp[])
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

hidden int __is_suid(const char *path)
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

hidden int __exec_sh(const char *path, char * const *argv, char *interparg[],
		     char *buf, size_t bufsiz)
{
	char *exec_sh = NULL;
	const char *origin;
	int i, err;

	/*
	 * Try in the iamroot origin path directory set by the environment
	 * variable IAMROOT_ORIGIN if set first, and in the iamroot library
	 * directory then.
	 */
	origin = _getenv("IAMROOT_ORIGIN");
	if (origin) {
		ssize_t siz;

		siz = __host_path_access("exec.sh", F_OK, origin, buf, bufsiz,
					 0);
		if (siz > 0)
			exec_sh = buf;
	}
	if (!exec_sh) {
		exec_sh = _strncpy(buf, __getexec(), bufsiz);

		err = next_faccessat(AT_FDCWD, exec_sh, F_OK, AT_EACCESS);
		if (err == -1)
			return -1;
	}
	/* Paranoid */
	if (!exec_sh)
		return __set_errno(ENOENT, -1);

	/*
	 * Run exec.sh from the host system.
	 *
	 * The library and the script exec.sh are host system binaries, it must
	 * be run from the host system. Furthermore, the intepreter for the
	 * script exec.sh is not necessarily part of the chroot environment.
	 */
	i = 0;
	interparg[i++] = exec_sh;
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

hidden struct kinfo_file *kinfo_getfile(pid_t pid, int *cntp)
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

hidden int __strtofd(const char *nptr, char **endptr)
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
hidden void __procfdname(char *buf, unsigned fd)
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

hidden ssize_t fpath(int fd, char *buf, size_t bufsiz)
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

hidden char *__fpath(int fd)
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

hidden char *__fpath2(int fd)
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
 * faccessat does not support flag AT_SYMLINK_NOFOLLOW; the glibc supports for
 * it, the others do not!
 */
static int __faccessat(int dfd, const char *path, int mode, int atflags)
{
	struct stat statbuf;

	if (atflags & AT_SYMLINK_NOFOLLOW)
		return fstatat(dfd, path, &statbuf, atflags & AT_EACCESS);

	return faccessat(dfd, path, mode, atflags & AT_SYMLINK_NOFOLLOW);
}

/*
 * Stolen and hacked from musl (src/process/execvp.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
hidden ssize_t __host_path_access(const char *file, int mode, const char *path,
				  char *buf, size_t bufsiz, off_t offset)
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

		if (next_faccessat(AT_FDCWD, b, mode, 0) != -1) {
			errno = 0;
			_strncpy(&buf[offset], b, bufsiz-offset);
			return strnlen(&buf[offset], bufsiz-offset);
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

/*
 * Stolen and hacked from musl (src/process/execvp.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
hidden ssize_t __path_access(const char *file, int mode, const char *path,
			     char *buf, size_t bufsiz)
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

		if (__faccessat(AT_FDCWD, b, mode, AT_SYMLINK_FOLLOW) != -1) {
			errno = 0;
			return path_resolution(AT_FDCWD, b, buf, bufsiz,
					       AT_SYMLINK_NOFOLLOW);
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

hidden int __path_iterate(const char *path,
			  int (*callback)(const char *, void *),
			  void *user)
{
	return __strtok(path, ":", callback, user);
}

hidden int __group_iterate(const char *path,
			   int (*callback)(const char *, void *),
			   void *user)
{
	return __strtok(path, " ", callback, user);
}

hidden int __dir_iterate(const char *path,
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

hidden char *__getroot()
{
	return _getenv("IAMROOT_ROOT");
}

hidden const char *__getexe()
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

hidden const char *__getfd(int fd)
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

hidden int __setfd(int fd, const char *path)
{
#if defined __OpenBSD__ || defined __NetBSD__
	const int errno_save = errno;
	char buf[NAME_MAX];
	int n;

	n = _snprintf(buf, sizeof(buf), "IAMROOT_FD_%i", fd);
	if (n == -1)
		return -1;

	__info("%i: %s='%s' -> '%s'\n", fd, buf, _getenv(buf), path);

	if (!path)
		return __set_errno(errno_save, _unsetenv(buf));

	return __set_errno(errno_save, _setenv(buf, path, 1));
#else
	(void)fd;
	(void)path;

	return 0;
#endif
}

hidden int __execfd()
{
	const int errno_save = errno;
#if defined __OpenBSD__ || defined __NetBSD__
	char * const *p;

	for (p = environ; *p; p++) {
		char path[PATH_MAX];
		int n, err, fd;

		n = sscanf(*p, "IAMROOT_FD_%i=%" __xstr(sizeof(PATH_MAX-1)) "s",
			   &fd, path);
		if (n != 2)
			continue;

		err = __unsetfd(fd);
		if (err == -1)
			__warning("%i: cannot unset fd!\n", fd);
	}
#endif

	return __set_errno(errno_save, 0);
}

#ifdef __linux__
hidden const char *__execfn()
{
	return (const char *)getauxval(AT_EXECFN);
}
#endif

#ifdef __FreeBSD__
hidden const char *__execfn()
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

hidden static char **get_proc_args()
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

hidden const char *__execfn()
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
hidden const char *__execfn()
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

hidden int __setrootdir(const char *path)
{
	if (!path) {
		__info("Exiting chroot: '%s'\n", __getrootdir());
		return _unsetenv("IAMROOT_ROOT");
	}

	__info("Enterring chroot: '%s'\n", path);
	return _setenv("IAMROOT_ROOT", path, 1);
}

hidden const char *__getrootdir()
{
	char *root;

	root = __getroot();
	if (!root)
		return "/";

	return root;
}

hidden int __chrootdir(const char *cwd)
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

hidden int __inchroot()
{
	return !streq(__getrootdir(), "/");
}

hidden const char *__skiprootdir(const char *path)
{
	const char *root;
	const char *ret;
	size_t len;

	if (!path || !*path)
		return __set_errno(EINVAL, NULL);

	root = __getrootdir();
	if (streq(root, "/"))
		return path;

	ret = path;
	len = __strlen(root);
	if (strneq(root, ret, len))
		ret = &ret[len];

	if (!*ret)
		ret = "/";

	return ret;
}

hidden char *__striprootdir(char *path)
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

hidden int __getno_color()
{
	return strtol(_getenv("NO_COLOR") ?: "0", NULL, 0);
}

hidden int __getcolor()
{
	const int errno_save = errno;

	if (!isatty(__getdebug_fd()))
		return __set_errno(errno_save, 0);

	return __getno_color() == 0;
}

hidden int __getdebug()
{
	return strtol(_getenv("IAMROOT_DEBUG") ?: "0", NULL, 0);
}

hidden int __getdebug_fd()
{
	return strtol(_getenv("IAMROOT_DEBUG_FD") ?: "2", NULL, 0);
}

static regex_t *re_ignore;

constructor void verbosef_init()
{
	static regex_t regex_ignore;
	const char *ignore;
	int ret;

	ignore = _getenv("IAMROOT_DEBUG_IGNORE");
	if (!ignore)
		ignore = "^$";

	ret = jim_regcomp(&regex_ignore, ignore, REG_EXTENDED);
	if (ret != 0) {
		jim_regex_perror("jim_regcomp", &regex_ignore, ret);
		return;
	}

	re_ignore = &regex_ignore;
}

destructor void verbosef_fini()
{
	if (!re_ignore)
		return;

	jim_regfree(re_ignore);
	re_ignore = NULL;
}

static int __ignore(const char *func)
{
	regmatch_t match;
	int ret = 0;

	if (!re_ignore)
		return 0;

	ret = jim_regexec(re_ignore, func, 1, &match, 0);
	if (ret == -1 || ret > REG_NOMATCH) {
		jim_regex_perror("jim_regexec", re_ignore, ret);
		return 0;
	}

	return !ret;
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int __vdverbosef(int fd, int lvl, const char *func, const char *fmt,
			va_list ap)
{
	int color, debug, err, ret = 0;
	const int errno_save = errno;

	if (lvl != 0 && __ignore(func))
		goto exit;

	debug = __getdebug();
	if (debug < lvl)
		goto exit;

	err = pthread_mutex_lock(&mutex);
	if (err)
		goto exit;

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
	fsync(fd);

	pthread_mutex_unlock(&mutex);

exit:
	return __set_errno(errno_save, ret);
}

hidden int __verbosef(int lvl, const char *func, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vdverbosef(DEBUG_FILENO, lvl, func, fmt, ap);
	va_end(ap);
	return ret;
}

#if !defined(NVERBOSE)
hidden void __verbose_execve(int lvl, char * const argv[], char * const envp[])
{
	const int errno_save = errno;
	int color, debug, err, fd;
	char * const *p;

	debug = __getdebug();
	if (debug < lvl)
		goto exit;

	fd = __getdebug_fd();
	if (fd < 0)
		goto exit;

	err = pthread_mutex_lock(&mutex);
	if (err)
		goto exit;

	color = __getcolor();
	if (color) {
		if (lvl == 0)
			dprintf(fd, "\033[31;1m");
		else
			dprintf(fd, "\033[32;1m");
	}

	dprintf(fd, "%s: ", lvl == 0 ? "Warning" : "Debug");

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
	fsync(fd);

	pthread_mutex_unlock(&mutex);

exit:
	errno = errno_save;
}
#endif

#ifdef __NetBSD__
#undef next_fstat
#endif

#ifdef __linux__

#if defined predict_false
# define __predict_false predict_false
#elif defined __glibc_unlikely
# define __predict_false __glibc_unlikely
#else
# define __predict_false(exp) (exp)
#endif

/*
 * Stolen from NetBSD (lib/libc/stdlib/reallocarr.c)
 *
 * SPDX-FileCopyrightText: The NetBSD Contributors
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/* $NetBSD: reallocarr.c,v 1.5 2015/08/20 22:27:49 kamil Exp $ */

/*-
 * Copyright (c) 2015 Joerg Sonnenberger <joerg@NetBSD.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <errno.h>
/* Old POSIX has SIZE_MAX in limits.h */
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SQRT_SIZE_MAX (((size_t)1) << (sizeof(size_t) * CHAR_BIT / 2))

int
reallocarr(void *ptr, size_t number, size_t size)
{
	int saved_errno, result;
	void *optr;
	void *nptr;

	saved_errno = errno;
	memcpy(&optr, ptr, sizeof(ptr));
	if (number == 0 || size == 0) {
		free(optr);
		nptr = NULL;
		memcpy(ptr, &nptr, sizeof(ptr));
		errno = saved_errno;
		return 0;
	}

	/*
	 * Try to avoid division here.
	 *
	 * It isn't possible to overflow during multiplication if neither
	 * operand uses any of the most significant half of the bits.
	 */
	if (__predict_false((number|size) >= SQRT_SIZE_MAX &&
	                    number > SIZE_MAX / size)) {
		errno = saved_errno;
		return EOVERFLOW;
	}

	nptr = realloc(optr, number * size);
	if (__predict_false(nptr == NULL)) {
		result = errno;
	} else {
		result = 0;
		memcpy(ptr, &nptr, sizeof(ptr));
	}
	errno = saved_errno;
	return result;
}
#endif
