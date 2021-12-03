/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_getcwd(char *buf, size_t size)
{
	char *(*sym)(char *, size_t);
	char *ret;

	sym = dlsym(RTLD_NEXT, "getcwd");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(buf, size);
	if (!ret)
		__perror(NULL, __func__);

	return ret;
}

char *getcwd(char *buf, size_t size)
{
	const char *root;
	size_t len;
	char *ret;

	ret = next_getcwd(buf, size);
	if (!ret) {
		perror(__func__);
		return NULL;
	}

	root = getrootdir();
	if (strcmp(root, "/") == 0)
		goto exit;

	len = strlen(root);
	if (strncmp(root, ret, len) == 0)
		__strlcpy(ret, &ret[len]);

	if (!*ret)
		strncpy(ret, "/", size);

exit:
	__verbose_func("%s(...)\n", __func__);

	return ret;
}
