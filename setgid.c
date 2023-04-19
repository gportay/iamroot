/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setgid(gid_t gid)
{
	char buf[BUFSIZ];
	int ret;

	__debug("%s(gid: %u)\n", __func__, gid);

	if (gid == (gid_t)-1)
		return __set_errno(EINVAL, -1);

	ret = _snprintf(buf, sizeof(buf), "%u", gid);
	if (ret == -1)
		return -1;

	return __setenv("GID", buf, 1);
}
