/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next___libc_start_main(int (*main)(int, char **, char **), int argc,
			   char **argv, int (*init)(int, char **, char **),
			   void (*fini)(void), void (*rtld_fini)(void),
			   void *stack_end)
{
	int (*sym)(int (*)(), int, char **, int (*)(int, char **, char **),
		   void (*)(), void(*)(), void(*)());
	int ret;

	sym = dlsym(RTLD_NEXT, "__libc_start_main");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(main, argc, argv, init, fini, rtld_fini, stack_end);
	if (!ret)
		__pathperror(NULL, __func__);

	return ret;
}

int __libc_start_main(int (*main)(int, char **, char **), int argc,
		      char **argv, int (*init)(int, char **, char **),
		      void (*fini)(void), void (*rtld_fini)(void),
		      void *stack_end)
{
	char *argv0;

	__debug("%s(main: %p, argc: %d, argv: { '%s', '%s', ... }, ...)\n",
		__func__, main, argc, argv[0], argv[1]);

	argv0 = getenv("argv0");
	if (argv0)
		argv[0] = argv0;

	return next___libc_start_main(main, argc, argv, init, fini, rtld_fini,
				      stack_end);
}
