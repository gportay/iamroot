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
#include <link.h>
#include <elf.h>

#include "iamroot.h"

#define __xstr(s) __str(s)
#define __str(s) #s

char *program;
char *argv0;
char *preload;
char *library_path;
int multiarch;
char *root;
char *cwd;
int debug;

static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
	const char **runpath = (const char **)data;
	int i;
	(void)size;

	if (!data)
		return __set_errno(EINVAL, -1);

	/*
	 * According to dl_iterate_phdr(3):
	 *
	 * The dl_iterate_phdr() function walks through the list of an
	 * application's shared objects and calls the function callback once
	 * for each object, until either all shared objects have been processed
	 * or callback returns a nonzero value.
	 *
	 * The first shared object for which output is displayed (where the
	 * name is an empty string) is the main program.
	 */
	if (__strnlen(info->dlpi_name) > 0)
		return 1;

	for (i = 0; i < info->dlpi_phnum; i++) {
		ElfW(Phdr) *phdr = (ElfW(Phdr) *)(&info->dlpi_phdr[i]);
		ElfW(Dyn) *dyn = (ElfW(Dyn) *)(info->dlpi_addr + phdr->p_vaddr);
		ElfW(Addr) strtab = 0;
		unsigned int j;
		const char *dt_runpath;

		if (info->dlpi_phdr[i].p_type != PT_DYNAMIC)
			continue;

		/* Look for the .strtab section */
		for (j = 0; j < info->dlpi_phdr[i].p_filesz / sizeof(Elf64_Dyn); j++) {
			if (dyn[j].d_tag != DT_STRTAB)
				continue;

			strtab = dyn[j].d_un.d_ptr;
			break;
		}

		/* No .strtab section */
		if (!strtab)
			continue;

		/* Look for the DT_RUNPATH tag */
		for (j = 0; j < info->dlpi_phdr[i].p_filesz / sizeof(Elf64_Dyn); j++) {
			if (dyn[j].d_tag != DT_RUNPATH)
				continue;

			dt_runpath = (const char *)(strtab + dyn[j].d_un.d_ptr);
			break;
		}

		if (!dt_runpath)
			continue;

		__info("DT_RUNPATH='%s'\n", dt_runpath);
		*runpath = dt_runpath;
		return 1;
	}

	return __set_errno(ENOENT, -1);
}

void usage(FILE * f, char * const arg0)
{
	fprintf(f, "Usage: %s [OPTIONS] [--] [PROGRAM [ARGUMENTS]]\n\n"
		   "Executes the PROGRAM in an iamroot environment ready to emulate the chroot(2) syscall for unprivileged users.\n\n"
		   "Options:\n"
		   " -A or --argv0 STRING      Set argv[0] to the value STRING before running the PROGRAM.\n"
		   " -P or --preload LIST      Preload the objects specified in LIST.\n"
		   " -L or --library-path PATH Use PATH instead of LD_LIBRARY_PATH environment variable setting.\n"
		   " -M or --multiarch         Use multiarch library path in chroot.\n"
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
		{ "multiarch",    no_argument,       NULL, 'M' },
		{ "root",         required_argument, NULL, 'R' },
		{ "debug",        no_argument,       NULL, 'D' },
		{ "version",      no_argument,       NULL, 'V' },
		{ "help",         no_argument,       NULL, 'h' },
		{ NULL,           no_argument,       NULL, 0   }
	};
	char *origin;
	int err;

	for (;;) {
		int index;
		int c = getopt_long(argc, argv, "A:P:L:MR:DVh", long_options, &index);
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

		case 'M':
			multiarch = 1;
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

	if (multiarch) {
		err = setenv("IAMROOT_MULTIARCH", "1", 1);
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

	origin = getenv("IAMROOT_ORIGIN");
	if (!origin) {
		err = dl_iterate_phdr(callback, &origin);
		if (err == -1 && errno == ENOENT) {
			origin = __xstr(PREFIX)"/lib/iamroot";
			__warning("%s: DT_RUNPATH is unset! assuming '%s'\n",
				  argv[0], origin);
		} else if (err == -1) {
			perror("dl_iterate_phdr");
			exit(EXIT_FAILURE);
		}
	}

	if (origin && *origin) {
		char buf[BUFSIZ];
		int n;

		err = setenv("IAMROOT_ORIGIN", origin, 1);
		if (err == -1) {
			perror("setenv");
			exit(EXIT_FAILURE);
		}

		n = snprintf(buf, sizeof(buf), "%s/libiamroot.so", origin);
		if (n == -1) {
			perror("snprintf");
			exit(EXIT_FAILURE);
		} else if ((size_t)n >= sizeof(buf)) {
			errno = ENOSPC;
			perror("snprintf");
			exit(EXIT_FAILURE);
		}

		err = setenv("LD_PRELOAD", buf, 1);
		if (err == -1) {
			perror("setenv");
			exit(EXIT_FAILURE);
		}
	}

#ifdef __FreeBSD__
	execvP(program, _getenv("PATH"), argv);
	perror("execvP");
#else
	execvpe(program, &argv[optind], __environ);
	perror("execvpe");
#endif
	exit(127);
}
