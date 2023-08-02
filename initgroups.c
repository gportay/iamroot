/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__
#include <unistd.h>
#endif

/*
 * Stolen from musl (src/misc/initgroups.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <grp.h>
#include <limits.h>

int initgroups(const char *user, gid_t gid)
{
        gid_t groups[NGROUPS_MAX];
        int count = NGROUPS_MAX;
        if (getgrouplist(user, gid, groups, &count) < 0) return -1;
        return setgroups(count, groups);
}
