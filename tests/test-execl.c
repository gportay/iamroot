/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <paths.h>

#include <unistd.h>

int main(void)
{
	execl(_PATH_BSHELL, "-sh", "-c", "echo \"$@\"", "sh", "one", "two",
	      "three", NULL);
	perror("execl");
	_exit(127);
}
