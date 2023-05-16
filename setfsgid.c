/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/fsuid.h>

#include "iamroot.h"

int setfsgid(gid_t fsgid)
{
	char buf[BUFSIZ];
	int ret;

	__debug("%s(fsgid: %u)\n", __func__, fsgid);

	if (fsgid == (gid_t)-1)
		return __set_errno(EINVAL, -1);

	ret = _snprintf(buf, sizeof(buf), "%u", fsgid);
	if (ret == -1)
		return -1;

	return __setenv("FSGID", buf, 1);
}
#endif
