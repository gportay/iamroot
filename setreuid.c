/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setreuid(uid_t ruid, uid_t euid)
{
	int ret;

	__debug("%s(ruid: %u, euid: %u)\n", __func__, ruid, euid);

	if (euid != (uid_t)-1) {
		ret = seteuid(euid);
		if (ret == -1)
			return -1;
	}

	if (ruid != (uid_t)-1) {
		ret = setuid(ruid);
		if (ret == -1)
			return -1;
	}

	return 0;
}
