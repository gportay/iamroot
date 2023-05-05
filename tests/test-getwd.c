/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <unistd.h>

#include "iamroot.h"

int main()
{
	char buf[PATH_MAX];
	char *cwd;

#if defined (__GLIBC__) || defined (__FreeBSD__)
	cwd = getwd(buf);
#else
	cwd = getcwd(buf, sizeof(buf));
#endif
	printf("%s\n", cwd);

	return EXIT_SUCCESS;
}
