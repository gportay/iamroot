/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setregid(gid_t rgid, gid_t egid)
{
	int ret;

	if (egid != (gid_t)-1) {
		ret = setegid(egid);
		if (ret == -1)
			goto exit;
	}

	if (rgid != (gid_t)-1) {
		ret = setgid(rgid);
		if (ret == -1)
			goto exit;
	}

	/* Not forwarding function */
	ret = 0;

exit:
	__debug("%s(rgid: %u, egid: %u) -> %i\n", __func__, rgid, egid, ret);

	return ret;
}
