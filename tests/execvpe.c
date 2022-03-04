/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>

int main(void)
{
	char * const argv[] = { "-sh", "-c", "echo \"$@\"", "sh", "one", "two",
				"three", NULL };

#ifdef __USE_GNU
	execvpe("sh", argv, environ);
#else
	execvp("sh", argv);
#endif
	_exit(127);
}
