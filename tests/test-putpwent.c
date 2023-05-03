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
#include <pwd.h>

#ifdef __linux__
int main(int argc, char * const argv[])
{
	struct passwd pwd;
	(void)argv;

	if (argc < 8) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 8) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	pwd.pw_name = argv[1];
	pwd.pw_passwd = argv[2];
	pwd.pw_uid = strtoul(argv[3], NULL, 0);
	pwd.pw_gid = strtoul(argv[4], NULL, 0);
	pwd.pw_gecos = argv[5];
	pwd.pw_dir = argv[6];
	pwd.pw_shell = argv[7];

	if (putpwent(&pwd, stdout)) {
		perror("putpwent");
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
