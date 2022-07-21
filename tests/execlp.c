/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>

#include <unistd.h>

int main(void)
{
	execlp("sh", "-sh", "-c", "echo \"$@\"", "sh", "one", "two", "three",
	       NULL);
	perror("execlp");
	_exit(127);
}
