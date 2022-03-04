/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <sys/mount.h>

#ifdef __linux__
struct options {
	int argc;
	char * const *argv;
	const char *type;
	unsigned long flags;
	const void *data;
};

static int getoptions(struct options *opts, int argc, char * const argv[])
{
	static const struct option long_options[] = {
		{ "type",  required_argument, NULL, 't' },
		{ NULL,    no_argument,       NULL, 0   }
	};

	opts->argc = argc;
	opts->argv = argv;
	opts->type = NULL;
	opts->flags = 0;
	opts->data = NULL;

	for (;;) {
		int index;
		int c = getopt_long(argc, argv, "t:", long_options, &index);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			opts->type = optarg;
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
	} else if (argc - argi < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc - argi > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (mount(argv[optind], argv[optind+1], options.type, options.flags,
		  options.data)) {
		perror("mount");
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
