/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dlfcn.h>

#define __dlperror(s) fprintf(stderr, "%s: %s\n", s, dlerror())

#ifdef __GLIBC__
int main(int argc, char * const argv[])
{
	int flags = RTLD_LAZY, ret = EXIT_FAILURE;
	const char *path = NULL;
	void *handle;

	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc > 1)
		path = argv[1];

	if (argc > 2)
		flags = strtoul(argv[2], NULL, 0);

	handle = dlmopen(LM_ID_BASE, path, flags);
	if (handle == NULL) {
		__dlperror("dlmopen");
		return ret;
	}

	ret = EXIT_SUCCESS;

	if (dlclose(handle))
		__dlperror("dlclose");

	return ret;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
