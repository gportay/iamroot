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

	__debug("%s(rgid: %u, egid: %u)\n", __func__, rgid, egid);

	if (egid != (gid_t)-1) {
		ret = setegid(egid);
		if (ret == -1)
			return -1;
	}

	if (rgid != (gid_t)-1) {
		ret = setgid(rgid);
		if (ret == -1)
			return -1;
	}

	return 0;
}
