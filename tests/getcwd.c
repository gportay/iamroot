/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <unistd.h>

#include "iamroot.h"

int main()
{
	char buf[PATH_MAX];
	char *cwd;

	cwd = getcwd(buf, sizeof(buf));
	printf("%s\n", cwd);

	return EXIT_SUCCESS;
}
