/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <grp.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_setgroups(size_t listsize, gid_t list[])
{
	int (*sym)(int, gid_t[]);
	int ret;

	sym = dlsym(RTLD_NEXT, "next_setgroups");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(listsize, list);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

int setgroups(size_t listsize, const gid_t *list)
{
	char buf[BUFSIZ];
	size_t size;
	size_t i;

	__debug("%s(listsize: %i, list: %p): IAMROOT_GROUPS: '%s'\n", __func__,
		(int)listsize, list, getenv("IAMROOT_GROUPS"));

	if (listsize == 0)
		return unsetenv("IAMROOT_GROUPS");

	if (listsize > NGROUPS_MAX || !list) {
		errno = EINVAL;
		return -1;
	}

	size = 0;
	for (i = 0; i < listsize; i++) {
		int n;

		n = _snprintf(&buf[size], sizeof(buf)-size, "%s%u",
			      i > 0 ? ":" : "", list[i]);
		if (n == -1)
			return -1;

		size += n;
	}

	return setenv("IAMROOT_GROUPS", buf, 1);
}
