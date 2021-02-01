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

char *get_current_dir_name()
{
	char *(*realsym)();
	char *root, *ret;
	size_t len;

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(...)\n", __func__);

	realsym = dlsym(RTLD_NEXT, __func__);
	if (!realsym) {
		errno = ENOTSUP;
		return NULL;
	}

	ret = realsym();
	if (!ret)
		return NULL;

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(ret: '%s', ...)\n", __func__, ret);

	root = getenv("IAMROOT_ROOT");
	if (!root)
		return ret;

	len = strlen(root);
	if (strncmp(root, ret, len) == 0)
		strcpy(ret, &ret[len]);

	if (!*ret)
		strcpy(ret, "/");

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(ret: '%s', ...)\n", __func__, ret);

	return ret;
}
