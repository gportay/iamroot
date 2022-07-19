/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <unistd.h>
#include <errno.h>

#include "iamroot.h"

int exect(const char *path, char *const argv[], char *const envp[])
{
	__debug("%s(path: '%s', argv: { '%s', '%s', ... }, envp: %p)\n",
		__func__, path, argv[0], argv[1], envp);

	return execve(path, argv, envp);
}
#endif
