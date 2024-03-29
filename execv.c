/*
 * Copyright 2020-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int execv(const char *path, char * const argv[])
{
	__debug("%s(path: '%s', argv: { '%s', '%s', ... })\n", __func__, path,
		argv[0], argv[1]);

	/* Forward to another function */
#ifdef __linux__
	return execvpe(path, argv, __environ);
#else
	return execvp(path, argv);
#endif
}
