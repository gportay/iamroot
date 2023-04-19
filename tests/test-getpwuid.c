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

int main(int argc, char * const argv[])
{
	struct passwd *pwd;
	uid_t uid;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	uid = strtoul(argv[1], NULL, 0);

	pwd = getpwuid(uid);
	if (!pwd) {
		perror("getpwuid");
		return EXIT_FAILURE;
	}

	printf("%s:%s:%u:%u:%s:%s:%s\n", pwd->pw_name, pwd->pw_passwd,
	       pwd->pw_uid, pwd->pw_gid, pwd->pw_gecos, pwd->pw_dir,
	       pwd->pw_shell);

	return EXIT_SUCCESS;
}
