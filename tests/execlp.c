/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>

int main(void)
{
	execlp("sh", "-sh", "-c", "echo \"$@\"", "sh", "one", "two",
	      "three", NULL);
	_exit(127);
}
