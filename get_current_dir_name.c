/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_get_current_dir_name()
{
	char *(*sym)();
	char *ret;

	sym = dlsym(RTLD_NEXT, "get_current_dir_name");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym();
	if (!ret)
		__pathperror(NULL, __func__);

	return ret;
}

char *get_current_dir_name()
{
	const char *root;
	size_t len, size;
	char *ret;

	ret = next_get_current_dir_name();
	if (!ret) {
		perror(__func__);
		return NULL;
	}

	root = getrootdir();
	if (strcmp(root, "/") == 0)
		goto exit;

	size = strlen(ret);
	len = strlen(root);
	if (strncmp(root, ret, len) == 0)
		memcpy(ret, &ret[len], len+1);

	if (!*ret)
		strncpy(ret, "/", size-1);

exit:
	__debug("%s()\n", __func__);

	return ret;
}
