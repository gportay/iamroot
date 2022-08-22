/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <link.h>

extern int next_dl_iterate_phdr(int (*)(struct dl_phdr_info *, size_t, void *),
				void *);

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

static int __dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size,
				      void *data)
{
	const char *root = (void *)data;
	const char *path = info->dlpi_name;
	char *val, *token, *saveptr;
	char buf[PATH_MAX];
	(void)size;

	__debug("%s(info: %p { .info->dlpi_name: '%s' }, ...)\n", __func__,
		info, info->dlpi_name);

	/* in chroot? */
	if ((*path != '/') || (__strlcmp(path, root) == 0))
		return 0;

	/* is an host interpreter? */
	if ((__strncmp(info->dlpi_name, "/lib/ld") == 0) ||
	    (__strncmp(info->dlpi_name, "/lib64/ld") == 0))
		return 0;

	/* is an IAMROOT_LIB? */
	val = getenv("IAMROOT_LIB");
	if (!val)
		return 0;
	val = __strncpy(buf, val);
	token = strtok_r(val, ":", &saveptr);
	if (!token) {
		if (__strlcmp(val, info->dlpi_name) == 0)
			return 0;
	} else if (*token) {
		do {
			if (__strlcmp(token, info->dlpi_name) == 0)
				return 0;
		} while ((token = strtok_r(NULL, ":", &saveptr)));
	}

	__warn_or_fatal("%s: is not in root directory '%s'\n", path, root);
	return 0;
}

int __libc_start_main(int (*main)(int, char **, char **), int argc,
		      char **argv, int (*init)(int, char **, char **),
		      void (*fini)(void), void (*rtld_fini)(void),
		      void *stack_end)
{
	const char *root;
	char *argv0;

	__debug("%s(main: %p, argc: %d, argv: { '%s', '%s', ... }, ...)\n",
		__func__, main, argc, argv[0], argv[1]);

	argv0 = getenv("argv0");
	if (argv0)
		argv[0] = argv0;

	root = getrootdir();
	if (!__streq(root, "/"))
		next_dl_iterate_phdr(__dl_iterate_phdr_callback, (void *)root);

	return next___libc_start_main(main, argc, argv, init, fini, rtld_fini,
				      stack_end);
}
