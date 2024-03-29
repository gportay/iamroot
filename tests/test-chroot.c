/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

extern ssize_t next_readlinkat(int, const char *, char *, size_t) weak;

static pid_t execvpf(const char *file, char * const argv[])
{
	pid_t pid;

	pid = fork();
	if (pid == -1)
		return -1;
	if (pid)
		return pid;

	close(STDIN_FILENO);
	open("/dev/null", O_RDONLY);

	execvp(file, argv);
	perror("execvp");
	_exit(127);
}

static int exec(const char *file, char * const argv[])
{
	int status;
	pid_t pid;

	pid = execvpf(file, argv);
	if (pid == -1)
		return -1;

	if (waitpid(pid, &status, 0) == -1)
		return -1;

	if (WIFEXITED(status) != 0)
		status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status) != 0)
		fprintf(stderr, "%s\n", strsignal(WTERMSIG(status)));

	return status;
}

static int in(const char *path)
{
	int fd;

	fd = open(".", O_RDONLY | O_CLOEXEC);
	if (fd == -1) {
		perror("open");
		return -1;
	}

	if (chroot(path) == -1)
		return -1;

	if (chdir("/") == -1) {
		perror("chdir");
		if (close(fd))
			perror("close");
		return -1;
	}

	return fd;
}

static int out(int fd)
{
	if (fchdir(fd)) {
		perror("fchdir");
		return -1;
	}

	if (close(fd))
		perror("close");

	return 0;
}

int main(int argc, char * const argv[])
{
	static char buf[PATH_MAX], cwd[PATH_MAX];
	int fd = -1, ret = EXIT_FAILURE;
	const char *root;
	ssize_t siz;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	}

	root = argv[1];
	fd = in(root);
	if (fd == -1) {
		perror("in");
		return EXIT_FAILURE;
	}

	if (argc > 2) {
		char *arg[argc-1];
		int i;

		arg[0] = argv[2];
		for (i = 0; i < argc-1; i++)
			arg[i+1] = argv[i+2];
		arg[argc-1] = NULL; /* NULL-terminated */

		ret = exec(arg[0], &arg[1]);
		if (ret == -1) {
			perror("exec");
			ret = EXIT_FAILURE;
		}
	}

	if (out(fd) == -1)
		perror("out");

	if (realpath(".", cwd) == NULL) {
		perror("realpath");
		goto exit;
	}

	siz = next_readlinkat(AT_FDCWD, "/proc/self/cwd", buf, sizeof(buf));
	if (siz == -1)
		goto exit;

	if (!streq(cwd, buf))
		goto exit;

	ret = EXIT_SUCCESS;

exit:
	return ret;
}
