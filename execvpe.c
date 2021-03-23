/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <unistd.h>

#define __strchrnul strchrnul

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern const char *getrootdir();

static int __execve(const char *path, char * const argv[], char * const envp[])
{
	const char *root;
	ssize_t len;

	root = getrootdir();
	if (strcmp(root, "/") == 0)
		goto exit;

	len = strlen(root);
	if (strncmp(path, root, len) == 0)
		path += len;

exit:
	return execve(path, argv, envp);
}

/* Stolen from musl (src/process/execvp.c) */
static int __execvpe(const char *file, char *const argv[], char *const envp[])
{
	const char *p, *z, *path = getenv("PATH");
	size_t l, k;
	int seen_eacces = 0;

	errno = ENOENT;
	if (!*file) return -1;

	if (strchr(file, '/'))
		return __execve(file, argv, envp);

	if (!path) path = "/usr/local/bin:/bin:/usr/bin";
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
		__execve(b, argv, envp);
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

int execvpe(const char *file, char * const argv[], char * const envp[])
{
	__fprintf(stderr, "%s(file: '%s', argv: '%s'...)\n", __func__, file,
			  argv[0]);

	return __execvpe(file, argv, envp);
}
