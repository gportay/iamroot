/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int setuid(uid_t uid)
{
	char buf[BUFSIZ];
	int ret;

	if (uid == (uid_t)-1) {
		ret = __set_errno(EINVAL, -1);
		goto exit;
	}

	ret = _snprintf(buf, sizeof(buf), "%u", uid);
	if (ret == -1)
		goto exit;

	/* Not forwarding function */
	ret = __setenv("UID", buf, 1);

exit:
	__debug("%s(uid: %u) -> %i\n", __func__, uid, ret);

	return ret;
}
