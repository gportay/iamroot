/*
 * Copyright 2021-2024 Gaël PORTAY
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
#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
	cwd = getwd(buf);
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
#else
	cwd = getcwd(buf, sizeof(buf));
#endif
	printf("%s\n", cwd);

	return EXIT_SUCCESS;
}
