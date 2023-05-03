/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifdef __linux__
#include <sys/types.h>
#include <shadow.h>

#define NUM(n) ((n) == -1 ? 0 : -1), ((n) == -1 ? 0 : (n))
#define STR(s) ((s) ? (s) : "")

int main(int argc, char * const argv[])
{
	struct spwd *spd;
	(void)argv;

	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 1) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	while ((spd = fgetspent(stdin)))
		printf("%s:%s:%.*li:%.*li:%.*li:%.*li:%.*li:%.*li:%.*lu\n",
		       STR(spd->sp_namp), STR(spd->sp_pwdp),
		       NUM(spd->sp_lstchg), NUM(spd->sp_min), NUM(spd->sp_max),
		       NUM(spd->sp_warn), NUM(spd->sp_inact),
		       NUM(spd->sp_expire), NUM((long)spd->sp_flag));
	if (errno && errno != ENOENT) {
		perror("fgetspent");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
