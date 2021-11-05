/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_setgid(gid_t gid)
{
	int (*sym)(gid_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "setgid");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(gid);
	if (ret == -1)
		__perror(NULL, __func__);

	return ret;
}

int setgid(gid_t gid)
{
	char buf[BUFSIZ];

	_snprintf(buf, sizeof(buf), "%u", gid);

	return setenv("IAMROOT_GID", buf, 1);
}
