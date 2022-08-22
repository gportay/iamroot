/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>

#include <sys/mount.h>

struct options {
	int argc;
	char * const *argv;
	int flags;
};

static int getoptions(struct options *opts, int argc, char * const argv[])
{
	static const struct option long_options[] = {
		{ "lazy",  required_argument, NULL, 'l' },
		{ "force", required_argument, NULL, 'f' },
		{ NULL,    no_argument,       NULL, 0   }
	};

	opts->argc = argc;
	opts->argv = argv;
	opts->flags = 0;

	for (;;) {
		int index;
		int c = getopt_long(argc, argv, "lf", long_options, &index);
		if (c == -1)
			break;

		switch (c) {
		case 'l':
#ifdef __linux__
			opts->flags |= MNT_DETACH;
#endif
			break;

		case 'f':
#ifdef __linux__
			opts->flags |= MNT_FORCE;
#endif
			break;

		default:
			errno = EINVAL;
			return -1;
		}
	}

	return optind;
}

int main(int argc, char * const argv[])
{
	static struct options options;
	int argi;

	argi = getoptions(&options, argc, argv);
	if (argi < 0) {
		perror("getoptions");
		exit(EXIT_FAILURE);
	} else if (argc - argi < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc - argi > 1) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

#ifdef __linux__
	if (umount2(argv[optind], options.flags)) {
		perror("umount2");
		return EXIT_FAILURE;
	}
#else
	(void)argv;
#endif

	return EXIT_SUCCESS;
}
