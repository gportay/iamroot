/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <link.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *),
			 void *data)
{
	int (*sym)(int (*)(struct dl_phdr_info *, size_t, void *), void *);

	sym = dlsym(RTLD_NEXT, "dl_iterate_phdr");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	return sym(callback, data);
}

struct dl_iterate_phdr_args {
	int (*callback)(struct dl_phdr_info *, size_t, void *);
	void *data;
};

static int __dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size,
				      void *data)
{
	struct dl_iterate_phdr_args *args;
	char buf[PATH_MAX];

	__strncpy(buf, info->dlpi_name);
	__striprootdir(buf);

	__debug("%s(info: %p { .info->dlpi_name: '%s' -> '%s'}, ...)\n",
		__func__, info, info->dlpi_name, buf);

	info->dlpi_name = buf;
	args = (struct dl_iterate_phdr_args *)data;
	return args->callback(info, size, args->data);
}

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *),
		    void *data)
{
	struct dl_iterate_phdr_args args = {
		.callback = callback,
		.data = data,
	};

	__debug("%s(callback: %p, data: %p)\n", __func__, callback, data);

	return next_dl_iterate_phdr(__dl_iterate_phdr_callback, &args);
}
