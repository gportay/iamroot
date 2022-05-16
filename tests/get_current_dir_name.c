/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include "iamroot.h"

int main()
{
	char *cwd = NULL;

#ifdef  __USE_GNU
	cwd = get_current_dir_name();
#endif
	if (!cwd)
		return EXIT_FAILURE;
	printf("%s\n", cwd);
	free(cwd);

	return EXIT_SUCCESS;
}
