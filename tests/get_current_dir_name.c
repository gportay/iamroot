/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#ifndef __USE_GNU
#include <string.h>
#include <limits.h>
#endif

#include <unistd.h>

#include "iamroot.h"

int main()
{
#ifndef __USE_GNU
	char buf[PATH_MAX];
#endif
	char *cwd = NULL;

#ifdef __USE_GNU
	cwd = get_current_dir_name();
#else
	cwd = getcwd(buf, sizeof(buf));
	if (!cwd)
		return EXIT_FAILURE;
	cwd = strdup(buf);
#endif
	if (!cwd)
		return EXIT_FAILURE;
	printf("%s\n", cwd);
	free(cwd);

	return EXIT_SUCCESS;
}
