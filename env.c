/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * GNU Bash supplies its own version of the environment functions getenv(),
 * _getenv(), putenv(), setenv(), and unsetenv(); the function clearenv() is
 * not supplied.
 *
 * As they said:
 *
 * 		We supply our own version of getenv () because we want library
 * 		routines to get the changed values of exported variables.
 *
 * The three environment functions putenv(), setenv() and unsetenv() do nothing
 * in the execve() context call: the two global variables environ and __environ
 * remain unchanged. However, the function clearenv() clears the environ and
 * set the two globals to NULL.
 *
 * All the environment functions are internaly supplied for the needs modifying
 * the environment, including LD_PRELOAD/LD_LIBRARY_PATH if the dynamic loader
 * do not support the options --preload/--library-path.
 *
 * It fixes the following error as LD_PRELOAD is STILL set to preload the GNU
 * libc in version 2.29 in the chroot environment, but exec.sh is run via the
 * /usr/bin/env linked against the GNU libc in version 2.38 in the host system:
 *
 * 	/usr/bin/env: /usr/lib64/libc-2.29.so: version `GLIBC_2.38' not found (required by /usr/bin/env)
 * 	/usr/bin/env: /usr/lib64/libc-2.29.so: version `GLIBC_2.34' not found (required by /usr/bin/env)
 * 	/usr/bin/env: /usr/lib64/libc-2.29.so: version `GLIBC_ABI_DT_RELR' not found (required by /usr/lib/libpthread.so.0)
 */

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <stdlib.h>

#include "iamroot.h"

#define clearenv _clearenv
#define getenv _getenv
#define putenv _putenv
#define setenv _setenv
#define unsetenv _unsetenv

void __env_rm_add(char *old, char *new);

/*
 * Stolen from musl (src/env/clearenv.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
hidden int clearenv()
{
	char **e = __environ;
	__environ = 0;
	if (e) while (*e) __env_rm_add(*e++, 0);
	return 0;
}

/*
 * Stolen from musl (src/env/getenv.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
hidden char *getenv(const char *name)
{
	size_t l = __strchrnul(name, '=') - name;
	if (l && !name[l] && __environ)
		for (char **e = __environ; *e; e++)
			if (!strncmp(name, *e, l) && l[*e] == '=')
				return *e + l+1;
	return 0;
}

/*
 * Stolen from musl (src/env/putenv.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
hidden int __putenv(char *s, size_t l, char *r)
{
	size_t i=0;
	if (__environ) {
		for (char **e = __environ; *e; e++, i++)
			if (!strncmp(s, *e, l+1)) {
				char *tmp = *e;
				*e = s;
				__env_rm_add(tmp, r);
				return 0;
			}
	}
	static char **oldenv;
	char **newenv;
	if (__environ == oldenv) {
		newenv = realloc(oldenv, sizeof *newenv * (i+2));
		if (!newenv) goto oom;
	} else {
		newenv = malloc(sizeof *newenv * (i+2));
		if (!newenv) goto oom;
		if (i) memcpy(newenv, __environ, sizeof *newenv * i);
		free(oldenv);
	}
	newenv[i] = s;
	newenv[i+1] = 0;
	__environ = oldenv = newenv;
	if (r) __env_rm_add(0, r);
	return 0;
oom:
	free(r);
	return -1;
}

hidden int putenv(char *s)
{
	size_t l = __strchrnul(s, '=') - s;
	if (!l || !s[l]) return unsetenv(s);
	return __putenv(s, l, 0);
}

/*
 * Stolen from musl (src/env/setenv.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
hidden void __env_rm_add(char *old, char *new)
{
	static char **env_alloced;
	static size_t env_alloced_n;
	for (size_t i=0; i < env_alloced_n; i++)
		if (env_alloced[i] == old) {
			env_alloced[i] = new;
			free(old);
			return;
		} else if (!env_alloced[i] && new) {
			env_alloced[i] = new;
			new = 0;
		}
	if (!new) return;
	char **t = realloc(env_alloced, sizeof *t * (env_alloced_n+1));
	if (!t) return;
	(env_alloced = t)[env_alloced_n++] = new;
}

hidden int setenv(const char *var, const char *value, int overwrite)
{
	char *s;
	size_t l1, l2;

	if (!var || !(l1 = __strchrnul(var, '=') - var) || var[l1]) {
		errno = EINVAL;
		return -1;
	}
	if (!overwrite && getenv(var)) return 0;

	l2 = strlen(value);
	s = malloc(l1+l2+2);
	if (!s) return -1;
	memcpy(s, var, l1);
	s[l1] = '=';
	memcpy(s+l1+1, value, l2+1);
	return __putenv(s, l1, s);
}

/*
 * Stolen from musl (src/env/unsetenv.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
int unsetenv(const char *name)
{
	size_t l = __strchrnul(name, '=') - name;
	if (!l || name[l]) {
		errno = EINVAL;
		return -1;
	}
	if (__environ) {
		char **e = __environ, **eo = e;
		for (; *e; e++)
			if (!strncmp(name, *e, l) && l[*e] == '=')
				__env_rm_add(*e, 0);
			else if (eo != e)
				*eo++ = *e;
			else
				eo++;
		if (eo != e) *eo = 0;
	}
	return 0;
}

#undef clearenv
#undef getenv
#undef putenv
#undef setenv
#undef unsetenv
