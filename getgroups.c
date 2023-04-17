/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

typedef struct groups {
	const int listsize;
	gid_t *list;
	int size;
} groups_t;

static int __id_callback(const char *id, void *user)
{
	groups_t *groups = (groups_t *)user;
	unsigned long ul;

	errno = 0;
	ul = strtoul(id, NULL, 0);
	if (errno)
		return -1;

	if (groups->listsize > 0 && !(groups->size < groups->listsize))
		return __set_errno(EINVAL, -1);

	if (groups->list && groups->size < groups->listsize)
		groups->list[groups->size] = ul;
	groups->size++;

	return 0;
}

__attribute__((visibility("hidden")))
int next_getgroups(int listsize, gid_t list[])
{
	int (*sym)(int, gid_t[]);
	int ret;

	sym = dlsym(RTLD_NEXT, "next_getgroups");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(listsize, list);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

int getgroups(int listsize, gid_t list[])
{
	groups_t groups = {
		.listsize = listsize,
		.list = list,
		.size = 0,
	};
	const char *val;
	int ret;

	if (listsize && !list)
		return __set_errno(EINVAL, -1);

	val = getenv("IAMROOT_GROUPS") ?: "0";
	ret = __path_iterate(val, __id_callback, &groups);
	if (ret == -1)
		return -1;

	__debug("%s(listsize: %i, list: %p): IAMROOT_GROUPS: '%s'\n", __func__,
		listsize, list, val);

	return groups.size;
}
