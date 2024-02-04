/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid)
{
	const int errno_save = errno;
	int ret = -1;

	if (ruid) {
		*ruid = getuid();
		if (*ruid == (uid_t)-1)
			goto exit;
	}

	if (euid) {
		*euid = geteuid();
		if (*euid == (uid_t)-1)
			goto exit;
	}

	if (suid) {
        	unsigned long ul;

		errno = 0;
		ul = strtoul(_getenv("IAMROOT_SUID") ?: "0", NULL, 0);
		if (errno)
			ul = 0;

		*suid = ul;
	}

	/* Not forwarding function */
	ret = __set_errno(errno_save, 0);

exit:
	__debug("%s(ruid: %p, euid: %p, suid: %p) -> %i\n", __func__, ruid,
		euid, suid, ret);

	return ret;
}
#endif
