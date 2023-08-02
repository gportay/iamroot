/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __linux__ || defined __OpenBSD__ || defined __NetBSD__
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
	*mem = calloc(sizeof(char *), *nmem+1);
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

	ret = n > nlim ? -1 : n;
	*ngroups = n;

cleanup:
	if (f) fclose(f);
	free(buf);
	free(mem);
	return ret;
}
#endif
