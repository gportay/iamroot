/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <limits.h>
#include <stdlib.h>

#include "path_resolution.h"

#define SYMLOOP_MAX 40
#define __strchrnul strchrnul
#define readlink next_readlink
#define getcwd next_getcwd
#define __strlcmp(s1, s2) strncmp(s1, s2, strlen(s2))

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern const char *getrootdir();
extern ssize_t next_readlink(const char *, char *, size_t);
extern char *next_getcwd(char *, size_t);

/* Stolen from musl (src/misc/realpath.c) */
static size_t slash_len(const char *s)
{
	const char *s0 = s;
	while (*s == '/') s++;
	return s-s0;
}

/* Stolen from musl (src/misc/realpath.c) */
char *next_realpath(const char *filename, char *resolved)
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
			errno = ENOENT;
			return 0;
		}
		if (k<0) {
			if (errno != EINVAL) return 0;
			errno = 0;
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

char *realpath(const char *path, char *resolved_path)
{
	char buf[PATH_MAX];
	const char *root;
	char *real_path;
	size_t len;
	char *ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	ret = next_realpath(real_path, resolved_path);
	if (!ret)
		goto exit;

	root = getrootdir();
	len = strlen(root);

	if (strcmp(root, "/") == 0)
		goto exit;

	if (__strlcmp(ret, getrootdir()) != 0)
		goto exit;

	if (!resolved_path) {
		resolved_path = strdup(ret+len);
		free(ret);
		ret = resolved_path;
		goto exit;
	}

	ret = memmove(resolved_path, ret+len, strlen(ret)-len+1);

exit:
	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return ret;

}
