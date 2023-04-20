/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ftw.h>

#ifndef __GLIBC__
#define stat64 stat
#define nftw64 nftw
#endif

/*
 * Stolen and hacked from man-pages (man3/ftw.3)
 *
 * SPDX-FileCopyrightText: Michael Kerrisk <mtk.manpages@gmail.com>
 *
 * SPDX-License-Identifier: Linux-man-pages-copyleft
 */
static int
display_info(const char *fpath, const struct stat64 *sb,
             int tflag, struct FTW *ftwbuf)
{
	printf("%-3s %2d ",
	       (tflag == FTW_D) ?   "d"   : (tflag == FTW_DNR) ? "dnr" :
	       (tflag == FTW_DP) ?  "dp"  : (tflag == FTW_F) ?   "f" :
	       (tflag == FTW_NS) ?  "ns"  : (tflag == FTW_SL) ?  "sl" :
	       (tflag == FTW_SLN) ? "sln" : "???",
	       ftwbuf->level);

	if (tflag == FTW_NS)
	    printf("-------");
	else
	    printf("%7jd", (intmax_t) sb->st_size);

	printf("   %-40s %d %s\n",
	       fpath, ftwbuf->base, fpath + ftwbuf->base);

	return 0;           /* To tell nftw64() to continue */
}

int
main(int argc, char *argv[])
{
	int flags = 0;

	if (argc > 2 && strchr(argv[2], 'd') != NULL)
	    flags |= FTW_DEPTH;
	if (argc > 2 && strchr(argv[2], 'p') != NULL)
	    flags |= FTW_PHYS;

	if (nftw64((argc < 2) ? "." : argv[1], display_info, 20, flags)
	    == -1)
	{
	    perror("nftw64");
	    exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
