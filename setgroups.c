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
	int ret = -1;
	size_t size;
	size_t i;

	if (listsize > NGROUPS_MAX) {
		ret = __set_errno(EINVAL, -1);
		goto exit;
	}

	if (listsize == 0) {
		ret = unsetenv("IAMROOT_GROUPS");
		goto exit;
	}

	size = 0;
	for (i = 0; i < listsize; i++) {
		int n;

		n = _snprintf(&buf[size], sizeof(buf)-size, "%s%u",
			      i > 0 ? ":" : "", list[i]);
		if (n == -1)
			goto exit;

		size += n;
	}

	/* Not forwarding function */
	ret = setenv("IAMROOT_GROUPS", buf, 1);

exit:
	__debug("%s(listsize: %i, list: %p): -> %i, IAMROOT_GROUPS: '%s'\n",
		__func__, (int)listsize, list, ret, getenv("IAMROOT_GROUPS"));

	return ret;
}
