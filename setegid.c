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
int next_setegid(gid_t gid)
{
	int (*sym)(gid_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "setegid");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(gid);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

int setegid(gid_t gid)
{
	char buf[BUFSIZ];
	int ret;

	__debug("%s(gid: %u)\n", __func__, gid);

	if (gid == (gid_t)-1)
		return __set_errno(EINVAL, -1);

	ret = _snprintf(buf, sizeof(buf), "%u", gid);
	if (ret == -1)
		return -1;

	return __setenv("EGID", buf, 1);
}
