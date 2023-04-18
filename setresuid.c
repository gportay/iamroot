/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	int ret;

	__debug("%s(ruid: %u, euid: %u, suid: %u)\n", __func__, ruid, euid,
		suid);

	ret = setreuid(ruid, euid);
	if (ret == -1)
		return ret;

	if (suid != (uid_t)-1) {
		char buf[BUFSIZ];

		ret = _snprintf(buf, sizeof(buf), "%u", suid);
		if (ret == -1)
			return -1;

		return __setenv("SUID", buf, 1);
	}

	return 0;
}
