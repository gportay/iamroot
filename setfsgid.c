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

	if (fsgid == (gid_t)-1) {
		ret = __set_errno(EINVAL, -1);
		goto exit;
	}

	ret = _snprintf(buf, sizeof(buf), "%u", fsgid);
	if (ret == -1)
		goto exit;

	/* Not forwarding function */
	ret = __setenv("FSGID", buf, 1);

exit:
	__debug("%s(fsgid: %u) -> %i\n", __func__, fsgid, ret);

	return ret;
}
#endif
