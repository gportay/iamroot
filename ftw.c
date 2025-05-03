/*
 * Copyright 2021-2023,2025 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif

/*
 * Stolen from musl (src/legacy/ftw.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <ftw.h>

int ftw(const char *path, int (*fn)(const char *, const struct stat *, int), int fd_limit)
{
	/* The following cast assumes that calling a function with one
	 * argument more than it needs behaves as expected. This is
	 * actually undefined, but works on all real-world machines. */
	return nftw(path, (int (*)())fn, fd_limit, FTW_PHYS);
}

#ifndef __clang__
#pragma GCC diagnostic pop
#endif
