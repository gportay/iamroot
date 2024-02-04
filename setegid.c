/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setegid(gid_t gid)
{
	char buf[BUFSIZ];
	int ret;

	if (gid == (gid_t)-1) {
		ret = __set_errno(EINVAL, -1);
		goto exit;
	}

	ret = _snprintf(buf, sizeof(buf), "%u", gid);
	if (ret == -1)
		goto exit;

	/* Not forwarding function */
	ret = _setenv("IAMROOT_EGID", buf, 1);

exit:
	__debug("%s(gid: %u) -> %i\n", __func__, gid, ret);

	return ret;
}
