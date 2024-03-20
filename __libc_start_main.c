/*
 * Copyright 2022-2024 Gaël PORTAY
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

#if defined __powerpc64__ && defined __GLIBC__
/*
 * Stolen from glibc (sysdeps/unix/sysv/linux/powerpc/libc-start.c)
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
struct startup_info
  {
    void *sda_base;
    int (*main) (int, char **, char **, void *);
    int (*init) (int, char **, char **, void *);
    void (*fini) (void);
  };

static int (*sym)(int, char **, char **, ElfW(auxv_t) *, void (*)(),
		  struct startup_info *, char **);

hidden int next___libc_start_main(int argc,
				  char **argv,
				  char **ev,
				  ElfW(auxv_t) *auxvec,
				  void (*rtld_fini)(void),
				  struct startup_info *stinfo,
				  char **stack_on_entry)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__libc_start_main");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(argc, argv, ev, auxvec, rtld_fini, stinfo, stack_on_entry);
}
#else
static int (*sym)(int (*)(), int, char **, int (*)(int, char **, char **),
		  void (*)(), void(*)(), void(*)());

hidden int next___libc_start_main(int (*main)(int, char **, char **),
				  int argc,
				  char **argv,
				  int (*init)(int, char **, char **),
				  void (*fini)(void),
				  void (*rtld_fini)(void),
				  void *stack_end)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__libc_start_main");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(main, argc, argv, init, fini, rtld_fini, stack_end);
}
#endif

static int __dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size,
				      void *data)
{
	const char *root = (void *)data;
	const char *path = info->dlpi_name;
	(void)size;

	__debug("%s(info: %p { .info->dlpi_name: '%s' }, ...)\n", __func__,
		info, info->dlpi_name);

	/* in chroot? */
	if ((*path != '/') || __strleq(path, root))
		return 0;

	/* is an host interpreter? */
	if (__strneq(path, "/lib/ld") || __strneq(path, "/lib64/ld"))
		return 0;

	/* is an iamroot library? */
	if (__strneq(__basename(path), "libiamroot"))
		return 0;

	__warn_or_fatal("%s: is not in root directory '%s'\n", path, root);
	return 0;
}

#if defined __powerpc64__ && defined __GLIBC__
int __libc_start_main(int argc,
		      char **argv,
		      char **ev,
		      ElfW(auxv_t) *auxvec,
		      void (*rtld_fini)(void),
		      struct startup_info *stinfo,
		      char **stack_on_entry)
{
	const char *root;
	char *argv0;


	__debug("%s(argc: %i, argv: { '%s', '%s', ... }, ...)\n", __func__,
		argc, argv[0], argv[1]);

	argv0 = _getenv("argv0");
	if (argv0)
		argv[0] = argv0;

	root = __getrootdir();
	if (!streq(root, "/"))
		next_dl_iterate_phdr(__dl_iterate_phdr_callback, (void *)root);

	return next___libc_start_main(argc, argv,  ev, auxvec, rtld_fini,
				      stinfo, stack_on_entry);
}
#else
int __libc_start_main(int (*main)(int, char **, char **), int argc,
		      char **argv, int (*init)(int, char **, char **),
		      void (*fini)(void), void (*rtld_fini)(void),
		      void *stack_end)
{
	const char *root;
	char *argv0;

	__debug("%s(main: %p, argc: %i, argv: { '%s', '%s', ... }, ...)\n",
		__func__, main, argc, argv[0], argv[1]);

	argv0 = _getenv("argv0");
	if (argv0)
		argv[0] = argv0;

	root = __getrootdir();
	if (!streq(root, "/"))
		next_dl_iterate_phdr(__dl_iterate_phdr_callback, (void *)root);

	return next___libc_start_main(main, argc, argv, init, fini, rtld_fini,
				      stack_end);
}
#endif
