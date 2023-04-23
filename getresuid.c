/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid)
{
	const int save_errno = errno;

	__debug("%s(ruid: %p, euid: %p, suid: %p)\n", __func__, ruid, euid,
		suid);

	if (ruid) {
		*ruid = getuid();
		if (*ruid == (uid_t)-1)
			return -1;
	}

	if (euid) {
		*euid = geteuid();
		if (*euid == (uid_t)-1)
			return -1;
	}

	if (suid) {
        	unsigned long ul;

		errno = 0;
		ul = strtoul(__getenv("SUID") ?: "0", NULL, 0);
		if (errno)
			ul = 0;

		*suid = ul;
	}

	return __set_errno(save_errno, 0);
}
