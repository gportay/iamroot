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

#include <fcntl.h>

#include "iamroot.h"

#define allow_magic_links(f) (((f) & PATH_RESOLUTION_NOMAGICLINKS) == 0)
#define allow_ignore(f) (((f) & PATH_RESOLUTION_NOIGNORE) == 0)
#define allow_walk_along(f) (((f) & PATH_RESOLUTION_NOWALKALONG) == 0)

/* AT flag FOLLOW takes precedence over NOFOLLOW */
#define follow_symlink(f) (((f) & AT_SYMLINK_FOLLOW) || ((f) & AT_SYMLINK_NOFOLLOW) == 0)

#include "jimregexp.h"

extern char *next_realpath(const char *, char *);

#ifndef SYMLOOP_MAX
#define SYMLOOP_MAX 40
#endif
#ifdef __linux__
#define getxattr next_getxattr
#endif
#if defined __FreeBSD__ || __NetBSD__
#define getxattr next_getxattr
#define extattr_get_file next_extattr_get_file
#endif
#define readlink(...) next_readlinkat(AT_FDCWD, __VA_ARGS__)
#define getcwd next_getcwd
#define strlen __strnlen

#ifdef __linux__
extern ssize_t next_getxattr(const char *, const char *, void *, size_t);
#endif
#if defined __FreeBSD__ || __NetBSD__
extern ssize_t next_extattr_get_file(const char *, int, const char *, void *,
				     size_t);
#endif
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
/* The original function name is realpath() in the musl sources */
static char *__realpathat(const char *filename, char *resolved, int atflags)
{
	char stack[PATH_MAX+1];
	char output[PATH_MAX];
	size_t p, q, l, l0, cnt=0, nup=0;
	int errno_save=errno;
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
		ssize_t k=-1;
		if (!follow_symlink(atflags))
			goto skip_readlink;
#ifdef __linux__
		k = getxattr(output, IAMROOT_XATTRS_PATH_RESOLUTION, stack, p);
		if (k<0) errno = errno_save;
#endif
#if defined __FreeBSD__ || __NetBSD__
		k = next_extattr_get_file(output, EXTATTR_NAMESPACE_USER,
					  IAMROOT_EXTATTR_PATH_RESOLUTION,
					  stack, p);
		if (k<0) errno = errno_save;
#endif
		if (k==(ssize_t)p) goto toolong;
		if (k>0) {
			memcpy(stack+p-k+1, stack, k-1);
			p -= k-1;
			goto restart;
		}
		k = readlink(output, stack, p);
		if (k==(ssize_t)p) goto toolong;
		if (!k) {
			if (errno == ENOENT) goto skip_readlink;
			errno = ENOENT;
			return 0;
		}
		if (k<0) {
			if (errno == ENOENT) goto skip_readlink;
			if (errno != EINVAL) return 0;
			errno = errno_save;
skip_readlink:
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
		if (*stack=='/' && __inchroot()) {
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
#if defined __FreeBSD__ || __NetBSD__
#undef extattr_get_file
#endif
#ifdef __linux__
#undef getxattr
#endif
#undef readlink
#undef getcwd
#undef strlen

static regex_t *re_ignore;
static regex_t *re_warning_ignore;

constructor void path_resolution_init()
{
	static regex_t regex_ignore, regex_warning_ignore;
	const char *ignore, *warning_ignore;
	int ret;

	if (re_ignore)
		goto warning_ignore;

	ignore = _getenv("IAMROOT_PATH_RESOLUTION_IGNORE");
	if (!ignore)
		ignore = "^(/proc/|/sys/|"_PATH_DEV"|"_PATH_VARRUN"|/run/)";

	ret = jim_regcomp(&regex_ignore, ignore, REG_EXTENDED);
	if (ret != 0) {
		jim_regex_perror("jim_regcomp", &regex_ignore, ret);
		return;
	}

	re_ignore = &regex_ignore;

warning_ignore:
	warning_ignore = _getenv("IAMROOT_PATH_RESOLUTION_WARNING_IGNORE");
	if (!warning_ignore)
		warning_ignore = "^/var/log/dnf\\.rpm\\.log$";

	ret = jim_regcomp(&regex_warning_ignore, warning_ignore, REG_EXTENDED);
	if (ret != 0) {
		jim_regex_perror("jim_regcomp", &regex_warning_ignore, ret);
		return;
	}

	re_warning_ignore = &regex_warning_ignore;
}

destructor void path_resolution_fini()
{
	/*
	 * Workaround: reset the root directory for later destructors call such
	 * as __gcov_exit().
	 */
	_unsetenv("IAMROOT_ROOT");

	if (!re_warning_ignore)
		goto ignore;

	jim_regfree(re_warning_ignore);
	re_warning_ignore = NULL;

ignore:
	if (!re_ignore)
		return;

	jim_regfree(re_ignore);
	re_ignore = NULL;
}

static int ignore(const char *path)
{
	regmatch_t match;
	int ret = 0;

	if (!re_ignore)
		return 0;

	if (!*path)
		return 0;

	ret = jim_regexec(re_ignore, path, 1, &match, 0);
	if (ret > REG_NOMATCH) {
		jim_regex_perror("jim_regexec", re_ignore, ret);
		return 0;
	}

	return !ret;
}

static int warning_ignore(const char *path)
{
	regmatch_t match;
	int ret = 0;

	if (!re_warning_ignore)
		return 0;

	ret = jim_regexec(re_warning_ignore, path, 1, &match, 0);
	if (ret > REG_NOMATCH) {
		jim_regex_perror("jim_regexec", re_warning_ignore, ret);
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

hidden int __path_ignored(int dfd, const char *path)
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
	return path_resolution2(dfd, path, buf, bufsiz, atflags, 0);
}

ssize_t path_resolution2(int dfd, const char *path, char *buf, size_t bufsiz,
			 int atflags, int prflags)
{
	const int errno_save = errno;
	char tmp[PATH_MAX+1];
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
	if (allow_magic_links(prflags) && streq(path, "/proc/1/root")) {
		__notice("%s: ignoring path resolution '%s'\n", __func__,
			 path);
		_strncpy(buf, "/", bufsiz);
		ret = strnlen(buf, bufsiz);
		if ((size_t)ret != __strnlen("/"))
			return __set_errno(ENAMETOOLONG, -1);
		return ret;
	}

	/*
	 * The files in /dev/fd/ are read by bash for process substitutions.
	 *
	 * According to bash(1):
	 *
	 *	Process Substitution
	 *
	 *	Process substitution allows a process's input or output to be
	 *	referred to using a filename. It takes the form of <(list) or
	 *	>(list). The process list is run asynchronously, and its input
	 *	or output appears as a filename. This filename is passed as an
	 *	argument to the current command as the result of the expansion.
	 *	If the >(list) form is used, writing to the file will provide
	 *	input for list. If the <(list) form is used, the file passed as
	 *	an argument should be read to obtain the output of list.
	 *	Process substitution is supported on systems that support named
	 *	pipes (FIFOs) or the /dev/fd method of naming open files.
	 *
	 * The path is left unresolved, and it is copied as is.
	 *
	 *	/usr/lib/rpm/sysusers.sh: line 122: /dev/fd/63: No such file or directory
	 */
	if (allow_magic_links(prflags) && __strneq(path, "/dev/fd/")) {
		__notice("%s: ignoring path resolution '%s'\n", __func__,
			 path);
		_strncpy(buf, path, bufsiz);
		ret = strnlen(buf, bufsiz);
		if ((size_t)ret != __strlen(path))
			return __set_errno(ENAMETOOLONG, -1);
		return ret;
	}

	/*
	 * The path is to be ignored.
	 *
	 * The path is copied as is.
	 */
	if (allow_ignore(prflags) && ignore(path)) {
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
				__warning("%s: contains root directory '%s'\n",
					  path, root);
			path = &path[len]; /* strip root directory */
		}

		n = _snprintf(buf, bufsiz, "%s%s", root, path);
		if (n == -1 && errno == ENOSPC)
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
		if (n == -1 && errno == ENOSPC)
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
	if (follow_symlink(atflags) && __realpathat(buf, tmp, atflags))
		_strncpy(buf, tmp, bufsiz);

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
	if (root && __strleq(buf, root) && allow_ignore(prflags) &&
	    ignore(buf+len))
		__striprootdir(buf);

	/*
	 * This resolves for symlinks and iamroot.path-resolution extended
	 * attribute.
	 */
	if (allow_walk_along(prflags) && __realpathat(buf, tmp, atflags))
		_strncpy(buf, tmp, bufsiz);

	return __set_errno(errno_save, strnlen(buf, bufsiz));
}

hidden char *__getpath(int dfd, const char *path, int atflags)
{
	static char buf[PATH_MAX];
	ssize_t siz;

	*buf = 0;
	if (__path_ignored(dfd, path) > 0) {
		if (!*path)
			path = __fpath(dfd);

		if (follow_symlink(atflags))
			return next_realpath(path, buf);

		return __strncpy(buf, path);
	}

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (!siz)
		return NULL;

	return buf;
}
