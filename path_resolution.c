/*
 * Copyright 2020-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <paths.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>

#include <fcntl.h>

#include "iamroot.h"

/* AT flag FOLLOW takes precedence over NOFOLLOW */
#define follow_symlink(f) ((f & AT_SYMLINK_FOLLOW) || (f & AT_SYMLINK_NOFOLLOW) == 0)

typedef struct {
	regex_t re;
#ifndef JIMREGEXP_H
	char jimpad[40];
#endif
} __regex_t;

extern char *next_realpath(const char *, char *);
extern ssize_t next_readlinkat(int, const char *, char *, size_t);
extern int next_scandir(const char *, struct dirent ***,
			int (*)(const struct dirent *),
			int (*)(const struct dirent **,
			const struct dirent **));
extern int next_lstat(const char *, struct stat *);

static int __getpath_resolution_workaround()
{
	return strtol(getenv("IAMROOT_PATH_RESOLUTION_WORKAROUND") ?: "0",
		      NULL, 0);
}

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

ssize_t __procfdreadlink(int fd, char *buf, size_t bufsize)
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

		ret = strlcpy(buf, kif[i].kf_path, bufsize);
		break;
	}

	free(kif);
	return ret;
}
#else
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

__attribute__((visibility("hidden")))
ssize_t __procfdreadlink(int fd, char *buf, size_t bufsize)
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
	ret = next_readlinkat(AT_FDCWD, tmp, buf, bufsize);
	errno = errno_save;
	return ret;
}
#endif

__attribute__((visibility("hidden")))
int __strtofd(const char *nptr, char **endptr)
{
	int save_errno;
	long l;

	save_errno = errno;
	errno = 0;
	l = strtol(nptr, endptr, 0);
	if (errno != 0)
		return -1;

	errno = save_errno;
	return l;
}

#define SYMLOOP_MAX 40
#define readlink(...) next_readlinkat(AT_FDCWD, __VA_ARGS__)
#define getcwd next_getcwd
#define strlen __strnlen

extern ssize_t next_readlinkat(int, const char *, char *, size_t);
extern char *next_getcwd(char *, size_t);

/*
 * Stolen from musl (src/misc/realpath.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static size_t slash_len(const char *s)
{
	const char *s0 = s;
	while (*s == '/') s++;
	return s-s0;
}

/*
 * Stolen and hacked from musl (src/misc/realpath.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
__attribute__((visibility("hidden")))
char *__realpath(const char *filename, char *resolved)
{
	char stack[PATH_MAX+1];
	char output[PATH_MAX];
	size_t p, q, l, l0, cnt=0, nup=0;
	int errno_save = errno;
	int check_dir=0;

	if (!filename) {
		errno = EINVAL;
		return 0;
	}
	l = strnlen(filename, sizeof stack);
	if (!l) {
		errno = ENOENT;
		return 0;
	}
	if (l >= PATH_MAX) goto toolong;
	p = sizeof stack - l - 1;
	q = 0;
	memcpy(stack+p, filename, l+1);

	/* Main loop. Each iteration pops the next part from stack of
	 * remaining path components and consumes any slashes that follow.
	 * If not a link, it's moved to output; if a link, contents are
	 * pushed to the stack. */
restart:
	for (; ; p+=slash_len(stack+p)) {
		/* If stack starts with /, the whole component is / or //
		 * and the output state must be reset. */
		if (stack[p] == '/') {
			check_dir=0;
			nup=0;
			q=0;
			output[q++] = '/';
			p++;
			/* Initial // is special. */
			if (stack[p] == '/' && stack[p+1] != '/')
				output[q++] = '/';
			continue;
		}

		char *z = __strchrnul(stack+p, '/');
		l0 = l = z-(stack+p);

		if (!l && !check_dir) break;

		/* Skip any . component but preserve check_dir status. */
		if (l==1 && stack[p]=='.') {
			p += l;
			continue;
		}

		/* Copy next component onto output at least temporarily, to
		 * call readlink, but wait to advance output position until
		 * determining it's not a link. */
		if (q && output[q-1] != '/') {
			if (!p) goto toolong;
			stack[--p] = '/';
			l++;
		}
		if (q+l >= PATH_MAX) goto toolong;
		memcpy(output+q, stack+p, l);
		output[q+l] = 0;
		p += l;

		int up = 0;
		if (l0==2 && stack[p-2]=='.' && stack[p-1]=='.') {
			up = 1;
			/* Any non-.. path components we could cancel start
			 * after nup repetitions of the 3-byte string "../";
			 * if there are none, accumulate .. components to
			 * later apply to cwd, if needed. */
			if (q <= 3*nup) {
				nup++;
				q += l;
				continue;
			}
			/* When previous components are already known to be
			 * directories, processing .. can skip readlink. */
			if (!check_dir) goto skip_readlink;
		}
		ssize_t k = readlink(output, stack, p);
		if (k==(ssize_t)p) goto toolong;
		if (!k) {
			if (errno == ENOENT) goto skip_readlink;
			errno = ENOENT;
			return 0;
		}
		if (k<0) {
			if (errno == ENOENT) goto skip_readlink;
			if (errno != EINVAL) return 0;
skip_readlink:
			errno = 0;
			check_dir = 0;
			if (up) {
				while(q && output[q-1]!='/') q--;
				if (q>1 && (q>2 || output[0]!='/')) q--;
				continue;
			}
			if (l0) q += l;
			check_dir = stack[p];
			continue;
		}
		if (++cnt == SYMLOOP_MAX) {
			errno = ELOOP;
			return 0;
		}

		/* If link is an absolute path, prepend root to resolve
		 * in chroot path. */
		if (*stack == '/' && __inchroot()) {
			const char *r = __getrootdir();
			ssize_t lr = __strlen(r);
			memmove(stack+lr, stack, k);
			memcpy(stack, r, lr);
			k += lr;
		}

		/* If link contents end in /, strip any slashes already on
		 * stack to avoid /->// or //->/// or spurious toolong. */
		if (stack[k-1]=='/') while (stack[p]=='/') p++;
		p -= k;
		memmove(stack+p, stack, k);

		/* Skip the stack advancement in case we have a new
		 * absolute base path. */
		goto restart;
	}

	output[q] = 0;

	if (output[0] != '/') {
		if (!getcwd(stack, sizeof stack)) return 0;
		l = strlen(stack);
		/* Cancel any initial .. components. */
		p = 0;
		while (nup--) {
			while(l>1 && stack[l-1]!='/') l--;
			if (l>1) l--;
			p += 2;
			if (p<q) p++;
		}
		if (q-p && stack[l-1]!='/') stack[l++] = '/';
		if (l + (q-p) + 1 >= PATH_MAX) goto toolong;
		memmove(output + l, output + p, q - p + 1);
		memcpy(output, stack, l);
		q = l + q-p;
	}

	errno = errno_save;
	if (resolved) return memcpy(resolved, output, q+1);
	else return strdup(output);

toolong:
	errno = ENAMETOOLONG;
	return 0;
}

#undef SYMLOOP_MAX
#undef readlink
#undef getcwd
#undef strlen

static regex_t *re_allow;
static regex_t *re_ignore;

static void __regex_perror(const char *s, regex_t *regex, int err)
{
	char buf[128];
	regerror(err, regex, buf, sizeof(buf));
	if (!s) {
		dprintf(STDERR_FILENO, "%s\n", buf);
		return;
	}

	dprintf(STDERR_FILENO, "%s: %s\n", s, buf);
}

__attribute__((constructor,visibility("hidden")))
void path_resolution_init()
{
	static __regex_t regex_allow, regex_ignore;
	const char *allow, *ignore;
	int ret;

	if (re_allow)
		goto ignore;

	allow = getenv("IAMROOT_PATH_RESOLUTION_ALLOW");
	if (!allow)
		allow = "^$";

	ret = regcomp(&regex_allow.re, allow, REG_NOSUB|REG_EXTENDED);
	if (ret) {
		__regex_perror("regcomp", &regex_allow.re, ret);
		return;
	}

	__info("IAMROOT_PATH_RESOLUTION_ALLOW=%s\n", allow);
	re_allow = &regex_allow.re;

ignore:
	ignore = getenv("IAMROOT_PATH_RESOLUTION_IGNORE");
	if (!ignore)
		ignore = "^/proc/|/sys/|"_PATH_DEV"|"_PATH_VARRUN"|/run/";

	ret = regcomp(&regex_ignore.re, ignore, REG_NOSUB|REG_EXTENDED);
	if (ret) {
		__regex_perror("regcomp", &regex_ignore.re, ret);
		return;
	}

	__info("IAMROOT_PATH_RESOLUTION_IGNORE=%s\n", ignore);
	re_ignore = &regex_ignore.re;
}

__attribute__((destructor,visibility("hidden")))
void path_resolution_fini()
{
	/*
	 * Workaround: reset the chroot directory for later destructors call
	 * such as __gcov_exit().
	 */
	unsetenv("IAMROOT_ROOT");

	if (!re_ignore)
		goto allow;

	regfree(re_ignore);
	re_ignore = NULL;

allow:
	if (!re_allow)
		return;

	regfree(re_allow);
	re_allow = NULL;
}

static int ignore(const char *path)
{
	int ret = 0;

	if (!re_allow)
		return 0;

	if (!re_ignore)
		return 0;

	if (!*path)
		return 0;

	ret = regexec(re_allow, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_allow, ret);
		return 0;
	}

	if (!ret)
		return 0;

	ret = regexec(re_ignore, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_ignore, ret);
		return 0;
	}

	return !ret;
}

char *__path_sanitize(char *path, size_t bufsize)
{
	ssize_t len;

	if (!*path)
		return path;

	len = strnlen(path, bufsize);
	while ((len > 3) && __strneq(path, "./") && path[2] != '/') {
		char *s;
		for (s = path; *s; s++)
			*s = *(s+2);
		len -= 2;
	}
	while ((len > 2) && __strneq(&path[len-2], "/.")) {
		path[len-2] = 0;
		len -= 2;
	}
	while ((len > 1) && (path[len-1] == '/')) {
		path[len-1] = 0;
		len--;
	}
	if (len == 0) {
		path[len++] = '.';
		path[len] = 0;
	}

	return path;
}

__attribute__((visibility("hidden")))
ssize_t fpath(int fd, char *buf, size_t bufsiz)
{
	ssize_t siz;

	if (fd < 0 && fd != AT_FDCWD)
		return __set_errno(EINVAL, -1);

	siz = __procfdreadlink(fd, buf, bufsiz);
	if (siz == -1)
		return -1;
	buf[siz] = 0; /* ensure NULL-terminated */

	return siz;
}

__attribute__((visibility("hidden")))
char *__fpath(int fd)
{
	const int save_errno = errno;
	static char buf[PATH_MAX];
	ssize_t siz;

	*buf = 0;
	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return __set_errno(save_errno, NULL);

	return buf;
}

__attribute__((visibility("hidden")))
char *__fpath2(int fd)
{
	const int save_errno = errno;
	static char buf[PATH_MAX];
	ssize_t siz;

	*buf = 0;
	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return __set_errno(save_errno, NULL);

	return buf;
}

__attribute__((visibility("hidden")))
int __path_ignored(int dfd, const char *path)
{
	if (dfd != AT_FDCWD) {
		char buf[PATH_MAX];
		ssize_t siz;

		siz = fpath(dfd, buf, sizeof(buf));
		if (siz == -1)
			return -1;

		return ignore(buf);
	}

	return ignore(path);
}

ssize_t path_resolution(int dfd, const char *path, char *buf, size_t bufsize,
			int atflags)
{
	const char *root;
	size_t len;

	if (!path)
		return __set_errno(EINVAL, -1);

	/*
	 * The files /proc/1/{cwd,root,exe} are readable by root only.
	 *
	 * The file /proc/1/root is often use to check if the process is in
	 * chroot.
	 *
	 * Assumes /proc/1/root is a symbolic link to / and resolves it
	 * manually.
	 *
	 *	root@archlinux:~$ stat /proc/1/root
	 *	  File: /proc/1/rootstat: cannot read symbolic link '/proc/1/root': Permission denied
	 *
	 *	  Size: 0         	Blocks: 0          IO Block: 1024   symbolic link
	 *	Device: 13h/19d	Inode: 2429        Links: 1
	 *	Access: (0777/lrwxrwxrwx)  Uid: (    0/    root)   Gid: (    0/    root)
	 *	Access: 2023-04-14 07:35:50.903331217 +0200
	 *	Modify: 2023-04-13 06:37:34.723333328 +0200
	 *	Change: 2023-04-13 06:37:34.723333328 +0200
	 *	 Birth: -
	 */
	if (streq(path, "/proc/1/root")) {
		__notice("%s: ignoring path resolution '%s'\n", __func__,
			 path);
		_strncpy(buf, "/", bufsize);
		return 1;
	}

	if (ignore(path))
		goto ignore;

	root = __getrootdir();
	if (streq(root, "/"))
		root = "";

	if (*path == '/') {
		if (*root && __strleq(path, root) &&
		    __getpath_resolution_workaround()) {
			__warn_or_fatal("%s: contains root directory '%s'\n",
					path, root);
			_strncpy(buf, path, bufsize);
		} else {
			int n;

			n = _snprintf(buf, bufsize, "%s%s", root, path);
			if (n < 0)
				return -1;
		}
	} else if (dfd != AT_FDCWD) {
		char dirbuf[PATH_MAX];
		ssize_t siz;
		int n;

		siz = fpath(dfd, dirbuf, sizeof(dirbuf));
		if (siz == -1)
			return __fpath_perror(dfd, -1);

		if (*dirbuf != '/')
			goto ignore;

		n = _snprintf(buf, bufsize, "%s/%s", dirbuf, path);
		if (n < 0)
			return -1;
	} else {
		_strncpy(buf, path, bufsize);
	}

	__path_sanitize(buf, bufsize);

	if (follow_symlink(atflags)) {
		char tmp[PATH_MAX];

		_strncpy(tmp, buf, bufsize);
		__realpath(tmp, buf);
	}

	len = __strlen(root);
	if (ignore(buf+len)) {
		memcpy(buf, buf+len, __strlen(buf+len)+1);
		goto exit;
	}

exit:
	return strnlen(buf, bufsize);

ignore:
	_strncpy(buf, path, bufsize);
	return strnlen(buf, bufsize);
}

__attribute__((visibility("hidden")))
char *__getpath(int dfd, const char *path, int atflags)
{
	static char buf[PATH_MAX];
	ssize_t siz;

	*buf = 0;
	if (__path_ignored(dfd, path) > 0) {
		if (!*path)
			path = __fpath(dfd);

		if (follow_symlink(atflags))
			return next_realpath(path, buf);

		return _strncpy(buf, path, sizeof(buf));
	}

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (!siz)
		return NULL;

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
