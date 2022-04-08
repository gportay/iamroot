/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>

#ifdef __FreeBSD__
extern char **environ;
#endif

int main(void)
{
	execle("sh", "-sh", "-c", "echo \"$@\"", "sh", "one", "two", "three",
	       NULL, environ);
	_exit(127);
}
