/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <pwd.h>

#include "iamroot.h"

#if defined __linux__ || defined __OpenBSD__ || defined __NetBSD__
static int _getpwnam_r(const char *, struct passwd *, char *, size_t,
		       struct passwd **);
static int _getpwuid_r(uid_t, struct passwd *, char *, size_t,
		       struct passwd **);
static struct passwd *_fgetpwent(FILE *);
static void _setpwent();
static struct passwd *_getpwent();
static struct passwd *_getpwuid(uid_t);
static struct passwd *_getpwnam(const char *);
static int _putpwent(const struct passwd *, FILE *);

int getpwnam_r(const char *name, struct passwd *pw, char *buf, size_t size,
	       struct passwd **res)
{
	int ret;

	ret = _getpwnam_r(name, pw, buf, size, res);

	__debug("%s(name: '%s', ...) -> %i\n", __func__, name, ret);

	return ret;
}

int getpwuid_r(uid_t uid, struct passwd *pw, char *buf, size_t size,
	       struct passwd **res)
{
	int ret;

	ret = _getpwuid_r(uid, pw, buf, size, res);

	__debug("%s(uid: %u, ...) -> %i\n", __func__, uid, ret);

	return ret;
}

struct passwd *fgetpwent(FILE *f)
{
	const int fd = fileno(f);
	struct passwd *ret;
	(void)fd;

	ret = _fgetpwent(f);

	__debug("%s(f: '%s') -> %p\n", __func__, __fpath(fd), ret);

	return ret;
}

void setpwent()
{
	_setpwent();
	__debug("%s()\n", __func__);
}

void endpwent()
{
	/* Forward to another function */
	_setpwent();
	__debug("%s()\n", __func__);
}

struct passwd *getpwent()
{
	struct passwd *ret;

	ret = _getpwent();

	__debug("%s() -> %p\n", __func__, ret);

	return ret;
}

struct passwd *getpwuid(uid_t uid)
{
	struct passwd *ret;

	ret = _getpwuid(uid);

	__debug("%s(uid: %u) -> %p\n", __func__, uid, ret);

	return ret;
}

struct passwd *getpwnam(const char *name)
{
	struct passwd *ret;

	ret = _getpwnam(name);

	__debug("%s(name: '%s') -> %p\n", __func__, name, ret);

	return ret;
}

int putpwent(const struct passwd *pw, FILE *f)
{
	const int fd = fileno(f);
	int ret;
	(void)fd;

	ret = _putpwent(pw, f);

	__debug("%s(..., f: '%s') -> %i\n", __func__, __fpath(fd), ret);

	return ret;
}

#define getpwnam_r _getpwnam_r
#define getpwuid_r _getpwuid_r
#define fgetpwent _fgetpwent
#define setpwent _setpwent
#define endpwent _endpwent
#define getpwent _getpwent
#define getpwuid _getpwuid
#define getpwnam _getpwnam
#define putpwent _putpwent

/*
 * Stolen from musl (src/include/features.h)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#define weak __attribute__((__weak__))
#define hidden __attribute__((__visibility__("hidden")))
#define weak_alias(old, new) \
	extern __typeof(old) new __attribute__((__weak__, __alias__(#old)))

/*
 * Stolen from musl (src/passwd/pwf.h)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

hidden int __getpwent_a(FILE *f, struct passwd *pw, char **line, size_t *size, struct passwd **res);
hidden int __getpw_a(const char *name, uid_t uid, struct passwd *pw, char **buf, size_t *size, struct passwd **res);
hidden int __getgrent_a(FILE *f, struct group *gr, char **line, size_t *size, char ***mem, size_t *nmem, struct group **res);
hidden int __getgr_a(const char *name, gid_t gid, struct group *gr, char **buf, size_t *size, char ***mem, size_t *nmem, struct group **res);

/*
 * Stolen and hacked from musl (src/passwd/getpwent_a.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <pthread.h>

static unsigned atou(char **s)
{
	unsigned x;
	for (x=0; **s-'0'<10; ++*s) x=10*x+(**s-'0');
	return x;
}

int __getpwent_a(FILE *f, struct passwd *pw, char **line, size_t *size, struct passwd **res)
{
	ssize_t l;
	char *s;
	int rv = 0;
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	for (;;) {
		if ((l=getline(line, size, f)) < 0) {
			rv = ferror(f) ? errno : 0;
			free(*line);
			*line = 0;
			pw = 0;
			break;
		}
		line[0][l-1] = 0;

		s = line[0];
		pw->pw_name = s++;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; pw->pw_passwd = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; pw->pw_uid = atou(&s);
		if (*s != ':') continue;

		*s++ = 0; pw->pw_gid = atou(&s);
		if (*s != ':') continue;

		*s++ = 0; pw->pw_gecos = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; pw->pw_dir = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; pw->pw_shell = s;
		break;
	}
	pthread_setcancelstate(cs, 0);
	*res = pw;
	if (rv) errno = rv;
	return rv;
}

/*
 * Stolen and hacked from musl (src/passwd/getpw_a.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <pthread.h>
#include <string.h>
#include <unistd.h>

int __getpw_a(const char *name, uid_t uid, struct passwd *pw, char **buf, size_t *size, struct passwd **res)
{
	FILE *f;
	int cs;
	int rv = 0;

	*res = 0;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

	f = fopen("/etc/passwd", "rbe");
	if (!f) {
		rv = errno;
		goto done;
	}

	while (!(rv = __getpwent_a(f, pw, buf, size, res)) && *res) {
		if ((name && !strcmp(name, (*res)->pw_name))
		|| (!name && (*res)->pw_uid == uid))
			break;
	}
	fclose(f);

done:
	pthread_setcancelstate(cs, 0);
	if (rv) errno = rv;
	return rv;
}

/*
 * Stolen from musl (src/passwd/getpw_r.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <pthread.h>

#define FIX(x) (pw->pw_##x = pw->pw_##x-line+buf)

static int getpw_r(const char *name, uid_t uid, struct passwd *pw, char *buf, size_t size, struct passwd **res)
{
	char *line = 0;
	size_t len = 0;
	int rv = 0;
	int cs;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

	rv = __getpw_a(name, uid, pw, &line, &len, res);
	if (*res && size < len) {
		*res = 0;
		rv = ERANGE;
	}
	if (*res) {
		memcpy(buf, line, len);
		FIX(name);
		FIX(passwd);
		FIX(gecos);
		FIX(dir);
		FIX(shell);
	}
	free(line);
	pthread_setcancelstate(cs, 0);
	if (rv) errno = rv;
	return rv;
}

int getpwnam_r(const char *name, struct passwd *pw, char *buf, size_t size, struct passwd **res)
{
	return getpw_r(name, 0, pw, buf, size, res);
}

int getpwuid_r(uid_t uid, struct passwd *pw, char *buf, size_t size, struct passwd **res)
{
	return getpw_r(0, uid, pw, buf, size, res);
}

/*
 * Stolen and hacked from musl (src/passwd/fgetpwent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
struct passwd *fgetpwent(FILE *f)
{
	static char *line;
	static struct passwd pw;
	size_t size=0;
	struct passwd *res;
	__getpwent_a(f, &pw, &line, &size, &res);
	if (!res && !errno) errno = ENOENT;
	return res;
}

/*
 * Stolen from musl (src/passwd/getpwent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static FILE *f;
static char *line;
static struct passwd pw;
static size_t size;

void setpwent()
{
	if (f) fclose(f);
	f = 0;
}

weak_alias(setpwent, endpwent);

struct passwd *getpwent()
{
	struct passwd *res;
	if (!f) f = fopen("/etc/passwd", "rbe");
	if (!f) return 0;
	__getpwent_a(f, &pw, &line, &size, &res);
	return res;
}

struct passwd *getpwuid(uid_t uid)
{
	struct passwd *res;
	__getpw_a(0, uid, &pw, &line, &size, &res);
	return res;
}

struct passwd *getpwnam(const char *name)
{
	struct passwd *res;
	__getpw_a(name, 0, &pw, &line, &size, &res);
	return res;
}

/*
 * Stolen from musl (src/passwd/putpwent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <pwd.h>
#include <stdio.h>

int putpwent(const struct passwd *pw, FILE *f)
{
	return fprintf(f, "%s:%s:%u:%u:%s:%s:%s\n",
		pw->pw_name, pw->pw_passwd, pw->pw_uid, pw->pw_gid,
		pw->pw_gecos, pw->pw_dir, pw->pw_shell)<0 ? -1 : 0;
}
#endif
