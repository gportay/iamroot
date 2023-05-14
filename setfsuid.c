/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/fsuid.h>

#include "iamroot.h"

int setfsuid(uid_t fsuid)
{
	char buf[BUFSIZ];
	int ret;

	if (fsuid == (uid_t)-1) {
		ret = __set_errno(EINVAL, -1);
		goto exit;
	}

	ret = _snprintf(buf, sizeof(buf), "%u", fsuid);
	if (ret == -1)
		goto exit;

	/* Not forwarding function */
	ret = __setenv("FSUID", buf, 1);

exit:
	__debug("%s(fsuid: %u) -> %i\n", __func__, fsuid, ret);

	return ret;
}
#endif
