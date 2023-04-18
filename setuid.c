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
int next_setuid(uid_t uid)
{
	int (*sym)(uid_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "setuid");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(uid);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

int setuid(uid_t uid)
{
	char buf[BUFSIZ];
	int ret;

	__debug("%s(uid: %u)\n", __func__, uid);

	if (uid == (uid_t)-1)
		return __set_errno(EINVAL, -1);

	ret = _snprintf(buf, sizeof(buf), "%u", uid);
	if (ret == -1)
		return -1;

	return __setenv("UID", buf, 1);
}
