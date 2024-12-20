/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <sys/types.h>
#include <grp.h>

#include "iamroot.h"

#if defined __linux__ || defined __OpenBSD__ || defined __NetBSD__
#define getgrnam_r _getgrnam_r
#define getgrgid_r _getgrgid_r
#define fgetgrent _fgetgrent
#define setgrent _setgrent
#define endgrent _endgrent
#define getgrent _getgrent
#define getgrgid _getgrgid
#define getgrnam _getgrnam
#define putgrent _putgrent
#define getgrouplist _getgrouplist
#ifdef __NetBSD__
#define getgroupmembership _getgroupmembership
#endif

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
 * Stolen and hacked from musl (src/passwd/getgrent_a.c)
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

int __getgrent_a(FILE *f, struct group *gr, char **line, size_t *size, char ***mem, size_t *nmem, struct group **res)
{
	ssize_t l;
	char *s, *mems;
	size_t i;
	int rv = 0;
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	for (;;) {
		if ((l=getline(line, size, f)) < 0) {
			rv = ferror(f) ? errno : 0;
			free(*line);
			*line = 0;
			gr = 0;
			goto end;
		}
		line[0][l-1] = 0;

		s = line[0];
		gr->gr_name = s++;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; gr->gr_passwd = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; gr->gr_gid = atou(&s);
		if (*s != ':') continue;

		*s++ = 0; mems = s;
		break;
	}

	for (*nmem=!!*s; *s; s++)
		if (*s==',') ++*nmem;
	free(*mem);
	*mem = calloc(*nmem+1,sizeof(char *));
	if (!*mem) {
		rv = errno;
		free(*line);
		*line = 0;
		gr = 0;
		goto end;
	}
	if (*mems) {
		mem[0][0] = mems;
		for (s=mems, i=0; *s; s++)
			if (*s==',') *s++ = 0, mem[0][++i] = s;
		mem[0][++i] = 0;
	} else {
		mem[0][0] = 0;
	}
	gr->gr_mem = *mem;
end:
	pthread_setcancelstate(cs, 0);
	*res = gr;
	if(rv) errno = rv;
	return rv;
}

/*
 * Stolen and hacked from musl (src/passwd/getgr_a.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <pthread.h>
#include <string.h>
#include <unistd.h>

int __getgr_a(const char *name, gid_t gid, struct group *gr, char **buf, size_t *size, char ***mem, size_t *nmem, struct group **res)
{
	FILE *f;
	int rv = 0;
	int cs;

	*res = 0;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	f = fopen("/etc/group", "rbe");
	if (!f) {
		rv = errno;
		goto done;
	}

	while (!(rv = __getgrent_a(f, gr, buf, size, mem, nmem, res)) && *res) {
		if ((name && !strcmp(name, (*res)->gr_name))
		|| (!name && (*res)->gr_gid == gid)) {
			break;
		}
	}
	fclose(f);

done:
	pthread_setcancelstate(cs, 0);
	if (rv) errno = rv;
	return rv;
}

/*
 * Stolen from musl (src/passwd/getgr_r.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <pthread.h>

#define FIX(x) (gr->gr_##x = gr->gr_##x-line+buf)

static int getgr_r(const char *name, gid_t gid, struct group *gr, char *buf, size_t size, struct group **res)
{
	char *line = 0;
	size_t len = 0;
	char **mem = 0;
	size_t nmem = 0;
	int rv = 0;
	size_t i;
	int cs;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

	rv = __getgr_a(name, gid, gr, &line, &len, &mem, &nmem, res);
	if (*res && size < len + (nmem+1)*sizeof(char *) + 32) {
		*res = 0;
		rv = ERANGE;
	}
	if (*res) {
		buf += (16-(uintptr_t)buf)%16;
		gr->gr_mem = (void *)buf;
		buf += (nmem+1)*sizeof(char *);
		memcpy(buf, line, len);
		FIX(name);
		FIX(passwd);
		for (i=0; mem[i]; i++)
			gr->gr_mem[i] = mem[i]-line+buf;
		gr->gr_mem[i] = 0;
	}
	free(mem);
	free(line);
	pthread_setcancelstate(cs, 0);
	if (rv) errno = rv;
	return rv;
}

int getgrnam_r(const char *name, struct group *gr, char *buf, size_t size, struct group **res)
{
	return getgr_r(name, 0, gr, buf, size, res);
}

int getgrgid_r(gid_t gid, struct group *gr, char *buf, size_t size, struct group **res)
{
	return getgr_r(0, gid, gr, buf, size, res);
}

/*
 * Stolen and hacked from musl (src/passwd/fgetgrent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
struct group *fgetgrent(FILE *f)
{
	static char *line, **mem;
	static struct group gr;
	struct group *res;
	size_t size=0, nmem=0;
	__getgrent_a(f, &gr, &line, &size, &mem, &nmem, &res);
	if (!res && !errno) errno = ENOENT;
	return res;
}

/*
 * Stolen from musl (src/passwd/getgrent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static FILE *f;
static char *line, **mem;
static struct group gr;

void setgrent()
{
	if (f) fclose(f);
	f = 0;
}

weak_alias(setgrent, endgrent);

struct group *getgrent()
{
	struct group *res;
	size_t size=0, nmem=0;
	if (!f) f = fopen("/etc/group", "rbe");
	if (!f) return 0;
	__getgrent_a(f, &gr, &line, &size, &mem, &nmem, &res);
	return res;
}

struct group *getgrgid(gid_t gid)
{
	struct group *res;
	size_t size=0, nmem=0;
	__getgr_a(0, gid, &gr, &line, &size, &mem, &nmem, &res);
	return res;
}

struct group *getgrnam(const char *name)
{
	struct group *res;
	size_t size=0, nmem=0;
	__getgr_a(name, 0, &gr, &line, &size, &mem, &nmem, &res);
	return res;
}

/*
 * Stolen from musl (src/passwd/putgrent.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <grp.h>
#include <stdio.h>

int putgrent(const struct group *gr, FILE *f)
{
	int r;
	size_t i;
	flockfile(f);
	if ((r = fprintf(f, "%s:%s:%u:", gr->gr_name, gr->gr_passwd, gr->gr_gid))<0) goto done;
	if (gr->gr_mem) for (i=0; gr->gr_mem[i]; i++)
		if ((r = fprintf(f, "%s%s", i?",":"", gr->gr_mem[i]))<0) goto done;
	r = fputc('\n', f);
done:
	funlockfile(f);
	return r<0 ? -1 : 0;
}

/*
 * Stolen an hacked from musl (src/passwd/getgrouplist.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <grp.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int getgrouplist(const char *user, gid_t gid, gid_t *groups, int *ngroups)
{
	int rv, nlim, ret = -1;
	ssize_t i, n = 1;
	struct group gr;
	struct group *res;
	FILE *f;
	char *buf = 0;
	char **mem = 0;
	size_t nmem = 0;
	size_t size;
	nlim = *ngroups;
	if (nlim >= 1) *groups++ = gid;

	f = fopen("/etc/group", "rbe");
	if (!f && errno != ENOENT && errno != ENOTDIR)
		goto cleanup;

	if (f) {
		while (!(rv = __getgrent_a(f, &gr, &buf, &size, &mem, &nmem, &res)) && res) {
			for (i=0; gr.gr_mem[i] && strcmp(user, gr.gr_mem[i]); i++);
			if (!gr.gr_mem[i]) continue;
			if (++n <= nlim) *groups++ = gr.gr_gid;
		}
		if (rv) {
			errno = rv;
			goto cleanup;
		}
	}

#ifndef __linux__
	/*
	 * According to getgrouplist(3):
	 *
	 * RETURN VALUE
	 *
	 * The getgrouplist() and getgroupmembership() functions return 0 if
	 * successful, and return -1 if the size of the group list is too small
	 * to hold all the user's groups.
	 */
	ret = n > nlim ? -1 : 0;
#else
	/*
	 * According to getgrouplist(3):
	 *
	 * RETURN VALUE
	 *
	 * If the number of groups of which user is a member is less than or
	 * equal to *ngroups, then the value *ngroups is returned.
	 *
	 * If the user is a member of more than *ngroups groups, then
	 * getgrouplist() returns -1. In this case, the value returned in
	 * *ngroups can be used to resize the buffer passed to a further call
	 * to getgrouplist().
	 */
	ret = n > nlim ? -1 : n;
#endif
	*ngroups = n;

cleanup:
	if (f) fclose(f);
	free(buf);
	free(mem);
	return ret;
}

#ifdef __NetBSD__
int getgroupmembership(const char *user, gid_t gid, gid_t *groups, int maxgrp,
		       int *ngroups)
{
	*ngroups = maxgrp;
	return getgrouplist(user, gid, groups, ngroups);
}
#endif

#undef getgrnam_r
#undef getgrgid_r
#undef fgetgrent
#undef setgrent
#undef endgrent
#undef getgrent
#undef getgrgid
#undef getgrnam
#undef putgrent
#undef getgrouplist
#ifdef __NetBSD__
#undef getgroupmembership
#endif

int getgrnam_r(const char *name, struct group *gr, char *buf, size_t size,
		struct group **res)
{
	int ret;

	ret = _getgrnam_r(name, gr, buf, size, res);

	__debug("%s(name: '%s', ...) -> %i\n", __func__, name, ret);

	return ret;
}

int getgrgid_r(gid_t gid, struct group *gr, char *buf, size_t size,
	       struct group **res)
{
	int ret;

	ret = _getgrgid_r(gid, gr, buf, size, res);

	__debug("%s(gid: %u, ...) -> %i\n", __func__, gid, ret);

	return ret;
}

struct group *fgetgrent(FILE *f)
{
	const int fd = fileno(f);
	struct group *ret;
	(void)fd;

	ret = _fgetgrent(f);

	__debug("%s(f: '%s') -> %p\n", __func__, __fpath(fd), ret);

	return ret;
}

void setgrent()
{
	_setgrent();
	__debug("%s()\n", __func__);
}

void endgrent()
{
	/* Forward to another function */
	_setgrent();
	__debug("%s()\n", __func__);
}

struct group *getgrent()
{
	struct group *ret;

	ret = _getgrent();

	__debug("%s() -> %p\n", __func__, ret);

	return ret;
}

struct group *getgrgid(gid_t gid)
{
	struct group *ret;

	ret = _getgrgid(gid);

	__debug("%s(gid: %u) -> %p\n", __func__, gid, ret);

	return ret;
}

struct group *getgrnam(const char *name)
{
	struct group *ret;

	ret = _getgrnam(name);

	__debug("%s(name: '%s') -> %p\n", __func__, name, ret);

	return ret;
}

int putgrent(const struct group *gr, FILE *f)
{
	const int fd = fileno(f);
	int ret;
	(void)fd;

	ret = _putgrent(gr, f);

	__debug("%s(..., f: '%s') -> %i\n", __func__, __fpath(fd), ret);

	return ret;
}

int getgrouplist(const char *user, gid_t gid, gid_t *groups, int *ngroups)
{
	int ret;

	ret = _getgrouplist(user, gid, groups, ngroups);

	__debug("%s(user: '%s', gid: %u, ...) -> %i\n", __func__, user, gid,
		ret);

	return ret;
}

#ifdef __NetBSD__
int getgroupmembership(const char *user, gid_t gid, gid_t *groups, int maxgrp,
		       int *ngroups)
{
	int ret;

	ret = _getgroupmembership(user, gid, groups, maxgrp, ngroups);

	__debug("%s(user: '%s', gid: %u, ...) -> %i\n", __func__, user, groups,
		ret);

	return ret;
}
#endif
#endif
