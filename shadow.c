/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <shadow.h>

#include "iamroot.h"

#define getspnam_r _getspnam_r
#define getspnam _getspnam
#define fgetspent _fgetspent
#define setspent _setspent
#define endspent _endspent
#define getspent _getspent
#define putspent _putspent
#define lckpwdf _lckpwdf
#define ulckpwdf _ulckpwdf

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
#include <shadow.h>
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
hidden int __parsespent(char *s, struct spwd *sp);

/*
 * Stolen and hacked from musl (src/passwd/getspnam_r.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>

/* This implementation support Openwall-style TCB passwords in place of
 * traditional shadow, if the appropriate directories and files exist.
 * Thus, it is careful to avoid following symlinks or blocking on fifos
 * which a malicious user might create in place of his or her TCB shadow
 * file. It also avoids any allocation to prevent memory-exhaustion
 * attacks via huge TCB shadow files. */

static long xatol(char **s)
{
	long x;
	if (**s == ':' || **s == '\n') return -1;
	for (x=0; **s-'0'<10; ++*s) x=10*x+(**s-'0');
	return x;
}

int __parsespent(char *s, struct spwd *sp)
{
	sp->sp_namp = s;
	if (!(s = strchr(s, ':'))) return -1;
	*s = 0;

	sp->sp_pwdp = ++s;
	if (!(s = strchr(s, ':'))) return -1;
	*s = 0;

	s++; sp->sp_lstchg = xatol(&s);
	if (*s != ':') return -1;

	s++; sp->sp_min = xatol(&s);
	if (*s != ':') return -1;

	s++; sp->sp_max = xatol(&s);
	if (*s != ':') return -1;

	s++; sp->sp_warn = xatol(&s);
	if (*s != ':') return -1;

	s++; sp->sp_inact = xatol(&s);
	if (*s != ':') return -1;

	s++; sp->sp_expire = xatol(&s);
	if (*s != ':') return -1;

	s++; sp->sp_flag = xatol(&s);
	if (*s != '\n') return -1;
	return 0;
}

static void cleanup(void *p)
{
	fclose(p);
}

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclobbered"
#endif

int getspnam_r(const char *name, struct spwd *sp, char *buf, size_t size, struct spwd **res)
{
	char path[20+NAME_MAX];
	FILE *f = 0;
	int rv = 0;
	int fd;
	size_t k, l = strlen(name);
	int skip = 0;
	int cs;
	int orig_errno = errno;

	*res = 0;

	/* Disallow potentially-malicious user names */
	if (*name=='.' || strchr(name, '/') || !l)
		return errno = EINVAL;

	/* Buffer size must at least be able to hold name, plus some.. */
	if (size < l+100)
		return errno = ERANGE;

	/* Protect against truncation */
	if (snprintf(path, sizeof path, "/etc/tcb/%s/shadow", name) >= (int)sizeof path)
		return errno = EINVAL;

	fd = open(path, O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
	if (fd >= 0) {
		struct stat st = { 0 };
		errno = EINVAL;
		if (fstat(fd, &st) || !S_ISREG(st.st_mode) || !(f = fdopen(fd, "rb"))) {
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
			close(fd);
			pthread_setcancelstate(cs, 0);
			return errno;
		}
	} else {
		if (errno != ENOENT && errno != ENOTDIR)
			return errno;
		f = fopen("/etc/shadow", "rbe");
		if (!f) {
			if (errno != ENOENT && errno != ENOTDIR)
				return errno;
			return 0;
		}
	}

	pthread_cleanup_push(cleanup, f);
	while (fgets(buf, size, f) && (k=strlen(buf))>0) {
		if (skip || strncmp(name, buf, l) || buf[l]!=':') {
			skip = buf[k-1] != '\n';
			continue;
		}
		if (buf[k-1] != '\n') {
			rv = ERANGE;
			break;
		}

		if (__parsespent(buf, sp) < 0) continue;
		*res = sp;
		break;
	}
	pthread_cleanup_pop(1);
	errno = rv ? rv : orig_errno;
	return rv;
}

#ifndef __clang__
#pragma GCC diagnostic pop
#endif

/*
 * Stolen from musl (src/passwd/getspnam.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#define LINE_LIM 256

struct spwd *getspnam(const char *name)
{
	static struct spwd sp;
	static char *line;
	struct spwd *res;
	int e;
	int orig_errno = errno;

	if (!line) line = malloc(LINE_LIM);
	if (!line) return 0;
	e = getspnam_r(name, &sp, line, LINE_LIM, &res);
	errno = e ? e : orig_errno;
	return res;
}

/*
 * Stolen and hacked from musl (src/passwd/fgetspent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <pthread.h>

struct spwd *fgetspent(FILE *f)
{
	static char *line;
	static struct spwd sp;
	size_t size = 0;
	struct spwd *res = 0;
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	if (getline(&line, &size, f) >= 0 && __parsespent(line, &sp) >= 0) res = &sp;
	if (!res && !errno) errno = ENOENT;
	pthread_setcancelstate(cs, 0);
	return res;
}

/*
 * Stolen from musl (src/passwd/getspent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
void setspent()
{
}

void endspent()
{
}

struct spwd *getspent()
{
	return 0;
}

/*
 * Stolen and hacked from musl (src/passwd/putspent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <shadow.h>
#include <stdio.h>

#define NUM(n) ((n) == -1 ? 0 : -1), ((n) == -1 ? 0 : (n))
#define STR(s) ((s) ? (s) : "")

int putspent(const struct spwd *sp, FILE *f)
{
	return fprintf(f, "%s:%s:%.*ld:%.*ld:%.*ld:%.*ld:%.*ld:%.*ld:%.*lu\n",
		STR(sp->sp_namp), STR(sp->sp_pwdp), NUM(sp->sp_lstchg),
		NUM(sp->sp_min), NUM(sp->sp_max), NUM(sp->sp_warn),
		NUM(sp->sp_inact), NUM(sp->sp_expire), NUM((long)sp->sp_flag)) < 0 ? -1 : 0;
}

/*
 * Stolen from musl (src/passwd/lckpwdf.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <shadow.h>

int lckpwdf()
{
	return 0;
}

int ulckpwdf()
{
	return 0;
}

#undef getspnam_r
#undef getspnam
#undef fgetspent
#undef setspent
#undef endspent
#undef getspent
#undef putspent
#undef lckpwdf
#undef ulckpwdf

int getspnam_r(const char *name, struct spwd *sp, char *buf, size_t size,
	       struct spwd **res)
{
	int ret;

	ret = _getspnam_r(name, sp, buf, size, res);

	__debug("%s(name: '%s', ...) -> %i\n", __func__, name, ret);

	return ret;
}

struct spwd *getspnam(const char *name)
{
	struct spwd *ret;

	ret = _getspnam(name);

	__debug("%s(name: '%s') -> %p\n", __func__, name, ret);

	return ret;
}

struct spwd *fgetspent(FILE *f)
{
	const int fd = fileno(f);
	struct spwd *ret;
	(void)fd;

	ret = _fgetspent(f);

	__debug("%s(f: '%s') -> %p\n", __func__, __fpath(fd), ret);

	return ret;
}

void setspent()
{
	_setspent();
	__debug("%s()\n", __func__);
}

void endspent()
{
	/* Forward to another function */
	_setspent();
	__debug("%s()\n", __func__);
}

struct spwd *getspent()
{
	struct spwd *ret;

	ret = _getspent();

	__debug("%s() -> %p\n", __func__, ret);

	return ret;
}

int putspent(const struct spwd *sp, FILE *f)
{
	const int fd = fileno(f);
	int ret;
	(void)fd;

	ret = _putspent(sp, f);

	__debug("%s(..., f: '%s') -> %i\n", __func__, __fpath(fd), ret);

	return ret;
}

int lckpwdf()
{
	int ret;

	ret = _lckpwdf();

	__debug("%s() -> %i\n", __func__, ret);

	return ret;
}

int ulckpwdf()
{
	int ret;

	ret = _ulckpwdf();

	__debug("%s() -> %i\n", __func__, ret);

	return ret;
}

#endif
