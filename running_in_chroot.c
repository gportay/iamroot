/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern const char *getrootdir();

__attribute__((visibility("hidden")))
int next_running_in_chroot(void)
{
	int (*sym)(void);

	sym = dlsym(RTLD_NEXT, "running_in_chroot");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym();
}

int running_in_chroot(void)
{
	__fprintf(stderr, "%s()\n", __func__);

	if (strcmp(getrootdir(), "/") != 0)                                     
		return 1;

	return next_running_in_chroot();
}
