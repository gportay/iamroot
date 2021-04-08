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

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern const char *getrootdir();

__attribute__((visibility("hidden")))
char *next_getwd(char *buf)
{
	char *(*sym)(char *);

	sym = dlsym(RTLD_NEXT, "getwd");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(buf);
}

char *getwd(char *buf)
{
	const char *root;
	size_t len;
	char *ret;

	ret = next_getwd(buf);
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
	__fprintf(stderr, "%s(...)\n", __func__);

	return ret;
}
