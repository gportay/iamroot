/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...);

char *next_getcwd(char *buf, size_t size)
{
	char *(*sym)(char *, size_t);

	sym = dlsym(RTLD_NEXT, "getcwd");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(buf, size);
}

char *getcwd(char *buf, size_t size)
{
	char *root, *ret;
	size_t len;

	ret = next_getcwd(buf, size);
	if (!ret) {
		perror(__func__);
		return NULL;
	}

	root = getenv("IAMROOT_ROOT");
	if (!root)
		goto exit;

	len = strlen(root);
	if (strncmp(root, ret, len) == 0)
		strcpy(ret, &ret[len]);

	if (!*ret)
		strcpy(ret, "/");

exit:
	__fprintf(stderr, "%s(...): IAMROOT_ROOT: '%s'\n", __func__, root);

	return ret;
}
