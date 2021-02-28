/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

extern const char *getrootdir();

__attribute__((visibility("hidden")))
char *next_get_current_dir_name()
{
	char *(*sym)();

	sym = dlsym(RTLD_NEXT, "get_current_dir_name");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
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
	if (strcmp(root, "/") == 0)
		goto exit;

	len = strlen(root);
	if (strncmp(root, ret, len) == 0)
		strcpy(ret, &ret[len]);

	if (!*ret)
		strcpy(ret, "/");

exit:
	__verbose("%s()\n", __func__);

	return ret;
}
