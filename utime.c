/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * Stolen from musl (src/time/utime.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <utime.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

int utime(const char *path, const struct utimbuf *times)
{
	return utimensat(AT_FDCWD, path, times ? ((struct timespec [2]){
		{ .tv_sec = times->actime }, { .tv_sec = times->modtime }})
		: 0, 0);
}
