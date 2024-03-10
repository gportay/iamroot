/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <fts.h>

int main(int argc, char * const argv[])
{
	int options = FTS_PHYSICAL, ret = EXIT_FAILURE;
	char *paths[] = { NULL, NULL };
	FTS *p;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 5) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc > 3)
		options = strtoul(argv[3], NULL, 0);

	paths[0] = argv[1];
	p = fts_open(paths, options, NULL);
	if (!p) {
		perror("fts_open");
		return EXIT_FAILURE;
	}

	for (;;) {
		FTSENT *e;

		e = fts_read(p);
		if (!e && !errno)
			break;
		if (!e) {
			perror("fts_read");
			goto exit;
		}

		printf("%s\n", e->fts_path);
	}

	ret = EXIT_SUCCESS;

exit:
	if (p)
		if (fts_close(p))
			perror("fts_close");

	return ret;
}
