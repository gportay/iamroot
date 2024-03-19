/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <getopt.h>

#include "iamroot.h"

#define __xstr(s) __str(s)
#define __str(s) #s

char *program;
char *argv0;
char *preload;
char *library_path;
char *root;
char *cwd;
int debug;

void usage(FILE * f, char * const arg0)
{
	fprintf(f, "Usage: %s [OPTIONS] [--] [PROGRAM [ARGUMENTS]]\n\n"
		   "Executes the PROGRAM in an iamroot environment ready to emulate the chroot(2) syscall for unprivileged users.\n\n"
		   "Options:\n"
		   " -A or --argv0 STRING      Set argv[0] to the value STRING before running the PROGRAM.\n"
		   " -P or --preload LIST      Preload the objects specified in LIST.\n"
		   " -L or --library-path PATH Use PATH instead of LD_LIBRARY_PATH environment variable setting.\n"
		   " -R or --root DIR          Set root directory to DIR.\n"
		   " -C or --cwd DIR           Set current working directory to DIR.\n"
		   " -D or --debug             Turn on debug mode.\n"
		   " -h or --help              Display this message.\n"
		   " -V or --version           Display the version.\n"
		   "", __basename(arg0));
}

int main(int argc, char * argv[])
{
	static const struct option long_options[] = {
		{ "argv0",        required_argument, NULL, 'A' },
		{ "preload",      required_argument, NULL, 'P' },
		{ "library-path", required_argument, NULL, 'L' },
		{ "root",         required_argument, NULL, 'R' },
		{ "debug",        no_argument,       NULL, 'D' },
		{ "version",      no_argument,       NULL, 'V' },
		{ "help",         no_argument,       NULL, 'h' },
		{ NULL,           no_argument,       NULL, 0   }
	};
	int err;

	for (;;) {
		int index;
		int c = getopt_long(argc, argv, "A:P:L:R:DVh", long_options, &index);
		if (c == -1)
			break;

		switch (c) {
		case 'A':
			argv0 = optarg;
			break;

		case 'P':
			preload = optarg;
			break;

		case 'L':
			library_path = optarg;
			break;

		case 'R':
			root = optarg;
			break;

		case 'C':
			cwd = optarg;
			break;

		case 'D':
			debug++;
			break;

		case 'V':
			printf("%s\n", __xstr(VERSION));
			exit(EXIT_SUCCESS);
			break;

		case 'h':
			usage(stdout, argv[0]);
			exit(EXIT_SUCCESS);
			break;

		case '?':
			exit(EXIT_FAILURE);
			break;

		default:
			fprintf(stderr, "Error: Illegal option code 0x%x!\n", c);
			exit(EXIT_FAILURE);
		}
	}

	if (argc - optind == 0) {
		usage(stdout, argv[0]);
		fprintf(stderr, "Error: Too few arguments!\n");
		exit(EXIT_FAILURE);
	}

	program = argv[optind];

	if (argv0)
		argv[optind] = argv0;

	if (preload) {
		err = setenv("LD_PRELOAD", preload, 1);
		if (err == -1) {
			perror("setenv");
			exit(EXIT_FAILURE);
		}
	}

	if (library_path) {
		err = setenv("LD_LIBRARY_PATH", library_path, 1);
		if (err == -1) {
			perror("setenv");
			exit(EXIT_FAILURE);
		}
	}

	if (root) {
		char buf[PATH_MAX];
		char *cwd;

		err = chdir(root);
		if (err == -1) {
			perror("chdir");
			exit(EXIT_FAILURE);
		}

		cwd = getcwd(buf, sizeof(buf));
		if (!cwd) {
			perror("cwd");
			exit(EXIT_FAILURE);
		}

		err = setenv("IAMROOT_ROOT", cwd, 1);
		if (err == -1) {
			perror("setenv");
			exit(EXIT_FAILURE);
		}
	}

	if (cwd) {
		err = chdir(cwd);
		if (err == -1) {
			perror("chdir");
			exit(EXIT_FAILURE);
		}
	}

	if (debug > 0) {
		char buf[BUFSIZ];
		int n;

		n = snprintf(buf, sizeof(buf), "%d", debug);
		if (n == -1) {
			perror("snprintf");
			exit(EXIT_FAILURE);
		} else if ((size_t)n >= sizeof(buf)) {
			errno = ENOSPC;
			perror("snprintf");
			exit(EXIT_FAILURE);
		}

		err = setenv("IAMROOT_DEBUG", buf, 1);
		if (err == -1) {
			perror("setenv");
			exit(EXIT_FAILURE);
		}
	}

	execvpe(program, &argv[optind], __environ);
	perror("execvpe");
	exit(127);
}
