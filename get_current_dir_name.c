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

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern const char *getrootdir();

char *next_get_current_dir_name()
{
	char *(*sym)();

	sym = dlsym(RTLD_NEXT, "get_current_dir_name");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym();
}

char *get_current_dir_name()
{
	const char *root;
	size_t len;
	char *ret;

	ret = next_get_current_dir_name();
	if (!ret) {
		perror(__func__);
		return NULL;
	}

	root = getrootdir();
	if (!root) {
		perror("getrootdir");
		return NULL;
	}

	if (strcmp(root, "/") == 0)
		goto exit;

	len = strlen(root);
	if (strncmp(root, ret, len) == 0)
		strcpy(ret, &ret[len]);

	if (!*ret)
		strcpy(ret, "/");

exit:
	__fprintf(stderr, "%s()\n", __func__);

	return ret;
}
