/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <paths.h>
#include <limits.h>

#include <unistd.h>

#include "iamroot.h"

#ifndef __FreeBSD__
extern int __execve(const char *, char * const [], char * const []);

/*
 * Stolen and hacked from musl (src/process/execvp.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static int __execvp(const char *file, char * const argv[])
{
	const char *p, *z, *path = getenv("PATH");
	size_t l, k;
	int seen_eacces = 0;

	errno = ENOENT;
	if (!*file) return -1;

	if (strchr(file, '/'))
		return __execve(file, argv, __environ);

	if (!path) path = _PATH_DEFPATH;
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
		__execve(b, argv, __environ);
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
#else
/*
 * Stolen from musl (src/process/execvp.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static int __execvP(const char *file, const char *path, char * const argv[])
{
	const char *p, *z;
	size_t l, k;
	int seen_eacces = 0;

	errno = ENOENT;
	if (!*file) return -1;

	if (strchr(file, '/'))
		return __execve(file, argv, environ);

	if (!path) path = _PATH_DEFPATH;
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
		__execve(b, argv, environ);
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

int execvP(const char *file, const char *path, char * const argv[])
{
	__debug("%s(file: '%s', path: '%s' argv: { '%s', '%s', ... })\n",
		__func__, file, path, argv[0], argv[1]);

	/* Forward to local function */
	return __execvP(file, path, argv);
}
#endif

int execvp(const char *file, char * const argv[])
{
	__debug("%s(file: '%s', argv: { '%s', '%s', ... })\n", __func__, file,
		argv[0], argv[1]);

#ifdef __FreeBSD__
	/* Forward to another function */
	return execvP(file, getenv("PATH"), argv);
#else
	/* Forward to local function */
	return __execvp(file, argv);
#endif
}
