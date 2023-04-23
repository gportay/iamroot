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

int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid)
{
	const int save_errno = errno;

	__debug("%s(rgid: %p, egid: %p, sgid: %p)\n", __func__, rgid, egid,
		sgid);

	if (rgid) {
		*rgid = getgid();
		if (*rgid == (gid_t)-1)
			return -1;
	}

	if (egid) {
		*egid = getegid();
		if (*egid == (gid_t)-1)
			return -1;
	}

	if (sgid) {
        	unsigned long ul;

		errno = 0;
		ul = strtoul(__getenv("SGID") ?: "0", NULL, 0);
		if (errno)
			ul = 0;

		*sgid = ul;
	}

	return __set_errno(save_errno, 0);
}
