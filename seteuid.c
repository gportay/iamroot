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
int next_seteuid(uid_t uid)
{
	int (*sym)(uid_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "seteuid");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(uid);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

int seteuid(uid_t uid)
{
	char buf[BUFSIZ];

	_snprintf(buf, sizeof(buf), "%u", uid);

	return setenv("IAMROOT_EUID", buf, 1);
}
