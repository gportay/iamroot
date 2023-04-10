/*
 * Copyright 2021-2023 Gaël PORTAY
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
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(uid);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

int seteuid(uid_t uid)
{
	char buf[BUFSIZ];
	int ret;

	ret = _snprintf(buf, sizeof(buf), "%u", uid);
	if (ret == -1)
		return -1;

	__debug("%s(uid: %u)\n", __func__, uid);

	return setenv("IAMROOT_EUID", buf, 1);
}
