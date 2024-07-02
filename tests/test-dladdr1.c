/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <link.h>

#include <dlfcn.h>

#define __dlperror(s) fprintf(stderr, "%s: %s\n", s, dlerror() ?: "(no error)")

#ifdef __GLIBC__
int main(int argc, char * const argv[])
{
	int flags = RTLD_LAZY, ret = EXIT_FAILURE;
	void *handle, *addr, *extra_info;
	struct link_map *l;
	Dl_info info;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	handle = dlopen(NULL, flags);
	if (handle == NULL) {
		__dlperror("dlopen");
		return ret;
	}

	addr = dlsym(handle, argv[1]);
	if (addr == NULL) {
		__dlperror("dlsym");
		goto exit;
	}

	if (dladdr1(addr, &info, &extra_info, RTLD_DL_LINKMAP) == 0) {
		__dlperror("dladdr1");
		goto exit;
	}

	l = (struct link_map *)extra_info;
	printf("%s:%s\n", info.dli_fname, l->l_name);

	ret = EXIT_SUCCESS;

exit:
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
