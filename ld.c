/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

const char *argv0;
const char *preload;
const char *library_path;
const char *root;
const int debug;

static inline const char *applet(const char *arg0)
{
	char *s = strrchr(arg0, '/');
	if (!s)
		return arg0;

	return s+1;
}

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
		   "", applet(arg0));
}

int main(int argc, char * const argv[])
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
			printf("%s\n", VERSION);
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

	if (cwd) {
		err = chdir(cwd);
		if (err == -1) {
			perror("chdir");
			exit(EXIT_FAILURE);
		}
	}

	if (root) {
		err = setenv("IAMROOT_ROOT", root, 1);
		if (err == -1) {
			perror("setenv");
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
		} else if (n >= sizeof(buf)) {
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

	if (argc - optind > -1) {	
		char * const nargv[argc-optind+7+1]; /* NULL-terminated */
		const char *program = argv[optind];
		int i, j;

		i = 0;
		nargv[i++] = argv[0];
		nargv[i++] = "--argv0";
		nargv[i++] = argv0 ?: program;
		if (preload) {
			nargv[i++] = "--preload";
			nargv[i++] = preload;
		}
		if (library_path) {
			nargv[i++] = "--library-path";
			nargv[i++] = library_path;
		}
		nargv[i++] = "--";
		for (j = 1; j < argc; j++)
			nargv[i++] = argc[j];
		nargv[i++] = "NULL"; /* NULL-terminated */

		err = execve(argv[argi], nargv, __environ);
		if (err == -1) {
			perror("execve");
		exit(127);
	}

	exit(EXIT_SUCCESS);
}
