/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
	const int errno_save = errno;
	int ret;

	ret = setregid(rgid, egid);
	if (ret == -1)
		goto exit;

	if (sgid != (uid_t)-1) {
		char buf[BUFSIZ];

		ret = _snprintf(buf, sizeof(buf), "%u", sgid);
		if (ret == -1)
			goto exit;

		ret = _setenv("IAMROOT_SGID", buf, 1);
		if (ret == -1)
			goto exit;
	}

	/* Not forwarding function */
	ret = __set_errno(errno_save, 0);

exit:
	__debug("%s(rgid: %u, egid: %u, sgid: %u) -> %i\n", __func__, rgid,
		egid, sgid, ret);

	return ret;
}
#endif
