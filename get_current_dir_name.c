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

char *get_current_dir_name()
{
	char *(*realsym)();
	char *root, *ret;
	size_t len;

	realsym = dlsym(RTLD_NEXT, __func__);
	if (!realsym) {
		errno = ENOTSUP;
		return NULL;
	}

	ret = realsym();
	if (!ret)
		return NULL;

	root = getenv("IAMROOT_ROOT");
	if (!root)
		goto exit;

	len = strlen(root);
	if (strncmp(root, ret, len) == 0)
		strcpy(ret, &ret[len]);

	if (!*ret)
		strcpy(ret, "/");

exit:
	__fprintf(stderr, "%s(): IAMROOT_ROOT: '%s'\n", __func__, root);

	return ret;
}
