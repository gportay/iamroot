/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <sys/fsuid.h>

#include "iamroot.h"

int setfsuid(uid_t fsuid)
{
	char buf[BUFSIZ];
	int ret;

	__debug("%s(fsuid: %u)\n", __func__, fsuid);

	if (fsuid == (uid_t)-1)
		return __set_errno(EINVAL, -1);

	ret = _snprintf(buf, sizeof(buf), "%u", fsuid);
	if (ret == -1)
		return -1;

	return __setenv("FSUID", buf, 1);
}
#endif
