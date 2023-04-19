/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <shadow.h>

#define NUM(n) ((n) == -1 ? 0 : -1), ((n) == -1 ? 0 : (n))
#define STR(s) ((s) ? (s) : "")

int main(int argc, char * const argv[])
{
	struct spwd *spd;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	spd = getspnam(argv[1]);
	if (!spd) {
		perror("getspnam");
		return EXIT_FAILURE;
	}

	printf("%s:%s:%.*li:%.*li:%.*li:%.*li:%.*li:%.*li:%.*lu\n",
	       STR(spd->sp_namp), STR(spd->sp_pwdp), NUM(spd->sp_lstchg),
	       NUM(spd->sp_min), NUM(spd->sp_max), NUM(spd->sp_warn),
	       NUM(spd->sp_inact), NUM(spd->sp_expire),
	       NUM((long)spd->sp_flag));

	return EXIT_SUCCESS;
}
