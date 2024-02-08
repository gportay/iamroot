/*
 * Copyright 2020-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <paths.h>
#include <limits.h>
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

#ifndef SYMLOOP_MAX
#define SYMLOOP_MAX 40
#endif
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

static regex_t *re_ignore;
static regex_t *re_warning_ignore;

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
	static __regex_t regex_ignore, regex_warning_ignore;
	const char *ignore, *warning_ignore;
	int ret;

	if (re_ignore)
		goto warning_ignore;

	ignore = _getenv("IAMROOT_PATH_RESOLUTION_IGNORE");
	if (!ignore)
		ignore = "^/proc/|/sys/|"_PATH_DEV"|"_PATH_VARRUN"|/run/";

	ret = regcomp(&regex_ignore.re, ignore, REG_NOSUB|REG_EXTENDED);
	if (ret == -1) {
		__regex_perror("regcomp", &regex_ignore.re, ret);
		return;
	}

	re_ignore = &regex_ignore.re;

warning_ignore:
	warning_ignore = _getenv("IAMROOT_PATH_RESOLUTION_WARNING_IGNORE");
	if (!warning_ignore)
		warning_ignore = "^/var/log/dnf\\.rpm\\.log$";

	ret = regcomp(&regex_warning_ignore.re, warning_ignore,
		      REG_NOSUB|REG_EXTENDED);
	if (ret == -1) {
		__regex_perror("regcomp", &regex_warning_ignore.re, ret);
		return;
	}

	re_warning_ignore = &regex_warning_ignore.re;
}

__attribute__((destructor,visibility("hidden")))
void path_resolution_fini()
{
	/*
	 * Workaround: reset the root directory for later destructors call such
	 * as __gcov_exit().
	 */
	_unsetenv("IAMROOT_ROOT");

	if (!re_warning_ignore)
		goto ignore;

	regfree(re_warning_ignore);
	re_warning_ignore = NULL;

ignore:
	if (!re_ignore)
		return;

	regfree(re_ignore);
	re_ignore = NULL;
}

static int ignore(const char *path)
{
	int ret = 0;

	if (!re_ignore)
		return 0;

	if (!*path)
		return 0;

	ret = regexec(re_ignore, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_ignore, ret);
		return 0;
	}

	return !ret;
}

static int warning_ignore(const char *path)
{
	int ret = 0;

	if (!re_warning_ignore)
		return 0;

	ret = regexec(re_warning_ignore, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_warning_ignore, ret);
		return 0;
	}

	return !ret;
}

/*
 * Sanitize the given path and make it relative to root (i.e. /).
 *
 * Important: it do **NOT** handle parent directories (i.e. ../).
 */
char *__path_sanitize(char *path, size_t bufsiz)
{
	ssize_t len;

	if (!*path)
		return path;

	len = strnlen(path, bufsiz);
	/* Strip leading ./ */
	while ((len > 3) && __strneq(path, "./") && path[2] != '/') {
		char *s;
		for (s = path; *(s+2); s++)
			*s = *(s+2);
		path[len-2] = 0; /* ensure NULL-terminated */
		len -= 2;
	}
	/* Strip trailing /. */
	while ((len > 2) && __strneq(&path[len-2], "/.")) {
		path[len-2] = 0; /* ensure NULL-terminated */
		len -= 2;
	}
	/* Strip trailing / */
	while ((len > 1) && (path[len-1] == '/')) {
		path[len-1] = 0; /* ensure NULL-terminated */
		len--;
	}
	/* Assuming . if reaches 0-length */
	if (len == 0) {
		path[len++] = '.';
		path[len] = 0;
	}

	return path;
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

ssize_t path_resolution(int dfd, const char *path, char *buf, size_t bufsiz,
			int atflags)
{
	int is_atrootfd = 0;
	const char *root;
	ssize_t ret;
	size_t len;

	if (!path)
		return __set_errno(EINVAL, -1);

	/*
	 * The path length exceeds the limit of PATH_MAX-1 bytes (i.e. the
	 * NULL-character **IS NOT** in the first PATH_MAX characters).
	 */
	len = strnlen(path, PATH_MAX);
	if (len == PATH_MAX)
		return __set_errno(ENAMETOOLONG, -1);

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
		_strncpy(buf, "/", bufsiz);
		ret = strnlen(buf, bufsiz);
		if ((size_t)ret != __strnlen("/"))
			return __set_errno(ENAMETOOLONG, -1);
		return ret;
	}

	/*
	 * The path is to be ignored.
	 *
	 * The path is copied as is.
	 */
	if (ignore(path)) {
		_strncpy(buf, path, bufsiz);
		ret = strnlen(buf, bufsiz);
		if ((size_t)ret != __strlen(path))
			return __set_errno(ENAMETOOLONG, -1);
		return ret;
	}

	/* Get the root directory and its length */
	root = __getrootdir();
	if (streq(root, "/"))
		root = "";
	len = __strlen(root);

	/*
	 * The path is an absolute path.
	 *
	 * The root directory is prepended to path.
	 */
	if (*path == '/') {
		int n;

		/* Warn if the path contains the root directory already */
		if (*root && __strleq(path, root)) {
			if (!warning_ignore(path+len))
				__warn_or_fatal("%s: contains root directory '%s'\n",
						path, root);
			path = &path[len]; /* strip root directory */
		}

		n = _snprintf(buf, bufsiz, "%s%s", root, path);
		if (n == -1 && n == ENOSPC)
			return __set_errno(ENAMETOOLONG, -1);
		if (n == -1)
			return -1;
	/*
	 * The path is a relative to dfd.
	 *
	 * The directory fd is resolved first, and it is prepended to path.
	 */
	} else if (dfd != AT_FDCWD) {
		char dirbuf[PATH_MAX];
		ssize_t siz;
		int n;

		/* Get the directory fd path */
		siz = fpath(dfd, dirbuf, sizeof(dirbuf));
		if (siz == -1)
			return -1;

		/*
		 * The directory fd is not an absolute path.
		 *
		 * The path is copied as is.
		 */
		if (*dirbuf != '/') {
			__warning("%i: ignore non absolute path '%s'\n", dfd,
				  dirbuf);
			_strncpy(buf, path, bufsiz);
			ret = strnlen(buf, bufsiz);
			if ((size_t)ret != __strlen(path))
				return __set_errno(ENAMETOOLONG, -1);
			return ret;
		}

		/*
		 * The directory fd is the root directory.
		 */
		is_atrootfd = streq(root, dirbuf);
		n = _snprintf(buf, bufsiz, "%s/%s", dirbuf, path);
		if (n == 1 && n == ENOSPC)
			return __set_errno(ENAMETOOLONG, -1);
		if (n == -1)
			return -1;
	/*
	 * The path is a relative path to cwd.
	 *
	 * The path is left unresolved, and it is copied as is.
	 */
	} else {
		_strncpy(buf, path, bufsiz);
	}

	/* Sanitize the resolved path if it is touched */
	__path_sanitize(buf, bufsiz);

	/* Follow the symlink unless the AT_SYMLINK_NOFOLLOW is given */
	if (follow_symlink(atflags)) {
		char tmp[PATH_MAX];

		_strncpy(tmp, buf, sizeof(tmp));
		__realpath(tmp, buf);
	}

	/*
	 * The directory fd is the root directory and the resolved path is
	 * below the root directory.
	 *
	 * The resolved path is the root directory.
	 */
	if (*root && is_atrootfd && !__strleq(buf, root))
		_strncpy(buf, root, bufsiz);

	/*
	 * The path after the symlinks are followed is to be ignored.
	 *
	 * The resolved path is stripped from the root directory.
	 */
	if (*root && __strleq(buf, root) && ignore(buf+len))
		__striprootdir(buf);

	return strnlen(buf, bufsiz);
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
