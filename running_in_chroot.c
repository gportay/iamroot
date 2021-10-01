/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include "iamroot.h"

extern int inchroot();

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
	__verbose("%s()\n", __func__);

	if (inchroot())
		return 1;

	return next_running_in_chroot();
}
