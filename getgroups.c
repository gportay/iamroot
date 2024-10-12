/*
 * Copyright 2022-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

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

	val = _getenv("IAMROOT_GROUPS") ?: "0";
	ret = __group_iterate(val, __id_callback, &groups);
	if (ret == -1)
		return -1;

	__debug("%s(listsize: %i, list: %p) -> %i, IAMROOT_GROUPS: '%s'\n",
		__func__, listsize, list, groups.size, val);

	return groups.size;
}

int __getgroups_chk(int listsize, gid_t list[], size_t listlen)
{
	(void)listlen;

	__debug("%s(listsize: %i, list: %p, listlen: %zu)\n", __func__,
		listsize, list, listlen);

	/* Forward to another function */
	return getgroups(listsize, list);
}
