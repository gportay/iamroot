/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include "iamroot.h"

int main()
{
	char *cwd;

	cwd = get_current_dir_name();
	printf("%s\n", cwd);
	free(cwd);

	return EXIT_SUCCESS;
}
