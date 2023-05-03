/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <shadow.h>

int main(int argc, char * const argv[])
{
	struct spwd spd;

	if (argc < 10) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 10) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	spd.sp_namp = argv[1];
	spd.sp_pwdp = argv[2];
	spd.sp_lstchg = strtol(argv[3], NULL, 0);
	spd.sp_min = strtol(argv[4], NULL, 0);
	spd.sp_max = strtol(argv[5], NULL, 0);
	spd.sp_warn = strtol(argv[6], NULL, 0);
	spd.sp_inact = strtol(argv[7], NULL, 0);
	spd.sp_expire = strtol(argv[8], NULL, 0);
	spd.sp_flag = strtoul(argv[9], NULL, 0);

	if (putspent(&spd, stdout)) {
		perror("putspent");
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
