/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <grp.h>

#include "iamroot.h"

int setgroups(size_t listsize, const gid_t *list)
{
	char buf[BUFSIZ];
	size_t size;
	size_t i;

	__debug("%s(listsize: %i, list: %p): IAMROOT_GROUPS: '%s'\n", __func__,
		(int)listsize, list, getenv("IAMROOT_GROUPS"));

	if (listsize == 0)
		return unsetenv("IAMROOT_GROUPS");

	if (listsize > NGROUPS_MAX || !list)
		return __set_errno(EINVAL, -1);

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
