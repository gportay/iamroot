/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_getwd(char *buf)
{
	char *(*sym)(char *);
	char *ret;

	sym = dlsym(RTLD_NEXT, "getwd");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(buf);
	if (!ret)
		__pathperror(NULL, __func__);

	return ret;
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
	if (__streq(root, "/"))
		goto exit;

	size = __strlen(ret);
	len = __strlen(root);
	if (strncmp(root, ret, len) == 0)
		memcpy(ret, &ret[len], __strlen(ret)-len+1); /* NUL */

	if (!*ret)
		strncpy(ret, "/", size-1);

exit:
	__debug("%s(buf: %p)\n", __func__, buf);

	return ret;
}
