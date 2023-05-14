/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <sys/stat.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
mode_t __getumask()
{
	return strtol(getenv("IAMROOT_UMASK") ?: "022", NULL, 8);
}

__attribute__((visibility("hidden")))
int __setumask(mode_t mask)
{
	char buf[4];
	int ret;

	ret = _snprintf(buf, sizeof(buf), "%o", mask);
	if (ret == -1)
		return -1;

	return setenv("IAMROOT_UMASK", buf, 1);
}

__attribute__((visibility("hidden")))
mode_t next_umask(mode_t mask)
{
	mode_t (*sym)(mode_t);

	sym = dlsym(RTLD_NEXT, "umask");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(mask);
}

mode_t umask(mode_t mask)
{
	mode_t ret, m;

	m = mask & ~(S_IRUSR | S_IWUSR);

	ret = __getumask();
	next_umask(m);
	__setumask(mask);

	__warn_if_too_restrictive_umask(mask);

	__debug("%s(mask: 0%03o -> 0%03o) -> %i\n", __func__, mask, m, ret);

	return ret;
}
