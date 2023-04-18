/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
	int ret;

	__debug("%s(rgid: %u, egid: %u, sgid: %u)\n", __func__, rgid, egid,
		sgid);

	ret = setregid(rgid, egid);
	if (ret == -1)
		return ret;

	if (sgid != (uid_t)-1) {
		char buf[BUFSIZ];

		ret = _snprintf(buf, sizeof(buf), "%u", sgid);
		if (ret == -1)
			return -1;

		return __setenv("SGID", buf, 1);
	}

	return 0;
}
