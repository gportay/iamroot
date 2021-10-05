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

__attribute__((visibility("hidden")))
char *next_getwd(char *buf)
{
	char *(*sym)(char *);

	sym = dlsym(RTLD_NEXT, "getwd");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	return sym(buf);
}

char *getwd(char *buf)
{
	const char *root;
	size_t len, size;
	char *ret;

	ret = next_getwd(buf);
	if (!ret) {
		perror(__func__);
		return NULL;
	}

	root = getrootdir();
	if (strcmp(root, "/") == 0)
		goto exit;

	size = strlen(buf);
	len = strlen(root);
	if (strncmp(root, ret, len) == 0)
		__strlcpy(ret, &ret[len]);

	if (!*ret)
		strncpy(ret, "/", size-1);

exit:
	__verbose("%s(...)\n", __func__);

	return ret;
}
