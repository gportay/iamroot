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

int seteuid(uid_t uid)
{
	char buf[BUFSIZ];
	int ret;

	__debug("%s(uid: %u)\n", __func__, uid);

	if (uid == (uid_t)-1)
		return __set_errno(EINVAL, -1);

	ret = _snprintf(buf, sizeof(buf), "%u", uid);
	if (ret == -1)
		return -1;

	return __setenv("EUID", buf, 1);
}
