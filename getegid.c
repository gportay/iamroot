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
gid_t next_getegid()
{
	gid_t (*sym)();
	gid_t ret;

	sym = dlsym(RTLD_NEXT, "getegid");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym();
	if (ret == (gid_t)-1)
		__pathperror(NULL, __func__);

	return ret;
}

gid_t getegid(void)
{
	unsigned long ul;

	errno = 0;
	ul = strtoul(getenv("IAMROOT_EGID") ?: "0", NULL, 0);
	if (errno)
		ul = 0;

	__debug("%s(): IAMROOT_EGID: %lu\n", __func__, ul);

	return ul;
}
