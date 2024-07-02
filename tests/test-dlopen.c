/*
 * Copyright 2022,2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dlfcn.h>

#define __dlperror(s) fprintf(stderr, "%s: %s\n", s, dlerror())

int main(int argc, char * const argv[])
{
	int flags = RTLD_LAZY, ret = EXIT_FAILURE;
	void *handle;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc > 2)
		flags = strtoul(argv[2], NULL, 0);

	handle = dlopen(argv[1], flags);
	if (handle == NULL) {
		__dlperror("dlopen");
		return ret;
	}

	ret = EXIT_SUCCESS;

	if (dlclose(handle))
		__dlperror("dlclose");

	return ret;
}
