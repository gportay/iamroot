/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <unistd.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...);

static int prependenv(const char *root, const char *name, const char *value,
		      int overwrite)
{
	char *old_value, *new_value = NULL, *tmp = NULL;
	char *token, *saveptr;
	size_t len = 0;
	int ret = -1;
	char *str;

	old_value = getenv(name);
	if (old_value && *old_value)
		len += strlen(old_value);

	tmp = strdup(value);
	if (!tmp)
		goto exit;

	len += strlen(tmp);

	token = strtok_r(tmp, ":", &saveptr);
	if (token && *token) {
		len += strlen(root);
		if (old_value && *old_value)
			len++;
		while ((token = strtok_r(NULL, ":", &saveptr)))
			len += strlen(root);
	}

	if (!len) {
		ret = 0;
		goto exit;
	}

	len++; /* NUL */
	new_value = malloc(len);
	if (!new_value)
		goto exit;

	*new_value = 0;
	str = new_value;

	strcpy(tmp, value);
	token = strtok_r(tmp, ":", &saveptr);
	if (token && *token) {
		int n;

		n = snprintf(str, len, "%s%s", root, token);
		str += n;
		len -= n;
		while ((token = strtok_r(NULL, ":", &saveptr))) {
			n = snprintf(str, len, ":%s%s", root, token);
			str += n;
			len -= n;
		}
	}

	if (old_value && *old_value) {
		int n;

		if (old_value && *old_value) {
		   *str++ = ':';
		   len--;
		}

		n = snprintf(str, len, "%s", old_value);
		str += n;
		len -= n;
	}

	ret = setenv(name, new_value, overwrite);

exit:
	free(tmp);
	free(new_value);

	return ret;
}

int chroot(const char *path)
{
	const char *real_path;
	char buf[PATH_MAX];

	/* prepend the current working directory for relative paths */
	if (path[0] != '/') {
		size_t len;
		char *cwd;

		cwd = getcwd(buf, sizeof(buf));
		len = strlen(cwd);
		if (len + 1 + strlen(path) + 1 > sizeof(buf)) {
			errno = ENAMETOOLONG;
			return -1;
		}

		cwd[len++] = '/';
		cwd[len] = 0;
		real_path = strncat(cwd, path, sizeof(buf) - len);
	} else {
		real_path = path_resolution(path, buf, sizeof(buf));
		if (!real_path) {
			perror("path_resolution");
			return -1;
		}
	}

	setenv("PATH", "/bin:/usr/bin", 1);
	prependenv(real_path, "LD_LIBRARY_PATH", "/usr/lib:/lib", 1);

	__fprintf(stderr, "%s(path: '%s' -> '%s') IAMROOT_ROOT='%s'\n",
			  __func__, path, real_path,
			  getenv("IAMROOT_ROOT") ?: "");

	return setenv("IAMROOT_ROOT", real_path, 1);
}
