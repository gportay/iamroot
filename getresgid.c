/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid)
{
	const int errno_save = errno;
	int ret = -1;

	if (rgid) {
		*rgid = getgid();
		if (*rgid == (gid_t)-1)
			goto exit;
	}

	if (egid) {
		*egid = getegid();
		if (*egid == (gid_t)-1)
			goto exit;
	}

	if (sgid) {
        	unsigned long ul;

		errno = 0;
		ul = strtoul(__getenv("SGID") ?: "0", NULL, 0);
		if (errno)
			ul = 0;

		*sgid = ul;
	}

	/* Not forwarding function */
	ret = __set_errno(errno_save, 0);

exit:
	__debug("%s(rgid: %p, egid: %p, sgid: %p) -> %i\n", __func__, rgid,
		egid, sgid, ret);

	return ret;
}
#endif
