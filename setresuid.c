/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	int ret;

	ret = setreuid(ruid, euid);
	if (ret == -1)
		goto exit;

	if (suid != (uid_t)-1) {
		char buf[BUFSIZ];

		ret = _snprintf(buf, sizeof(buf), "%u", suid);
		if (ret == -1)
			goto exit;

		ret = __setenv("SUID", buf, 1);
	}

	/* Not forwarding function */
	ret = 0;

exit:
	__debug("%s(ruid: %u, euid: %u, suid: %u) -> %i\n", __func__, ruid,
		euid, suid, ret);

	return ret;
}
#endif
