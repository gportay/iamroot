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

/*
 * Stolen and hacked from man-pages (man3/ftw.3)
 *
 * SPDX-FileCopyrightText: Michael Kerrisk <mtk.manpages@gmail.com>
 *
 * SPDX-License-Identifier: Linux-man-pages-copyleft
 */
static int
display_info(const char *fpath, const struct stat *sb,
             int tflag)
{
	printf("%-3s",
	       (tflag == FTW_D) ?   "d"   : (tflag == FTW_DNR) ? "dnr" :
	       (tflag == FTW_DP) ?  "dp"  : (tflag == FTW_F) ?   "f" :
	       (tflag == FTW_NS) ?  "ns"  : (tflag == FTW_SL) ?  "sl" :
	       (tflag == FTW_SLN) ? "sln" : "???");

	if (tflag == FTW_NS)
	    printf("-------");
	else
	    printf("%7jd", (intmax_t) sb->st_size);

	printf("   %-40s\n",
	       fpath);

	return 0;           /* To tell ftw() to continue */
}

int
main(int argc, char *argv[])
{
	if (ftw((argc < 2) ? "." : argv[1], display_info, 20)
	    == -1)
	{
	    perror("ftw");
	    exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
