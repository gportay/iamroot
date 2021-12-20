/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>

int main(void)
{
	execle("sh", "-sh", "-c", "echo \"$@\"", "sh", "one", "two", "three",
	       NULL, environ);
	_exit(127);
}
