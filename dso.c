/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <link.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#ifdef __NetBSD__
#include <sys/exec_elf.h>
#endif
#include <spawn.h>

#include "iamroot.h"

#include "jimregexp.h"

#ifndef ELFOSABI_GNU
#define ELFOSABI_GNU ELFOSABI_LINUX
#endif

extern int next_faccessat(int, const char *, int, int);
#ifdef __NetBSD__
#define next_fstat next___fstat50
extern int next___fstat50(int, struct stat *);
#else
extern int next_fstat(int, struct stat *);
#endif
extern int next_open(const char *, int, mode_t);
extern void *next_dlopen(const char *, int);

static int __getmultiarch()
{
	return strtol(_getenv("IAMROOT_MULTIARCH") ?: "0", NULL, 0);
}

/*
 * Stolen from musl (include/byteswap.h)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static uint16_t __bswap_16__(uint16_t __x)
{
	return __x<<8 | __x>>8;
}

static uint32_t __bswap_32__(uint32_t __x)
{
	return __x>>24 | (__x>>8&0xff00) | (__x<<8&0xff0000) | __x<<24;
}

static uint64_t __bswap_64__(uint64_t __x)
{
	return (__bswap_32__(__x)+0ULL)<<32 | __bswap_32__(__x>>32);
}

static ssize_t __getld_library_path(char *buf, size_t bufsiz, off_t offset)
{
	char *curr, *prev, *s;

	/* LD_LIBRARY_PATH is unset; return NULL directly! */
	curr = _getenv("LD_LIBRARY_PATH");
	if (!curr) {
		buf[offset] = 0;
		goto exit;
	}

	/*
	 * ld_library_path is unset; i.e. ld.so has --library-path or bootstrap
	 * environment.
	 *
	 * Return LD_LIBRARY_PATH's value.
	 */
	prev = _getenv("ld_library_path");
	if (!prev) {
		_strncpy(buf+offset, curr, bufsiz-offset);
		goto exit;
	}

	/*
	 * Both values are identicals; i.e. ld.so does not have --library-path,
	 * and LD_LIBRARY_PATH is left untouched; it is "empty" with no initial
	 * value!
	 *
	 * Return empty string.
	 */
	if (streq(curr, prev)) {
		buf[offset] = 0;
		goto exit;
	}

	/*
	 * Both values are differents; i.e. ld.so does not have --library-path,
	 * and LD_LIBRARY_PATH **WAS** touched!
	 */

	/*
	 * ld_library_path is **NOT** a substring of LD_LIBRARY_PATH; i.e it
	 * has reset!
	 *
	 * Return LD_LIBRARY_PATH's value.
	 */
	s = strstr(curr, prev);
	if (!s) {
		_strncpy(buf+offset, curr, bufsiz-offset);
		goto exit;
	}

	/*
	 * ld_library_path is a substring of LD_LIBRARY_PATH; i.e. it has been
	 * touched!
	 */

	/* Strip leading ld_library_path! */
	if (s == curr) {
		size_t len;

		len = __strlen(prev);
		if (s[len] == ':')
			s++;
		_strncpy(buf+offset, s+len, bufsiz-offset);
		goto exit;
	}

	/* Strip off ld_library_path from LD_LIBRARY_PATH! */
	__notice("LD_LIBRARY_PATH: '%s' contains '%s'\n", curr, prev);
	_strncpy(buf+offset, curr, bufsiz-offset);

exit:
	return strnlen(buf+offset, bufsiz-offset);
}

static int __setld_preload(const char *preload, int overwrite)
{
	char buf[PATH_MAX];
	char *curr;
	int n;

	if (!preload || !*preload)
		return __set_errno_and_perror(EINVAL, -1);

	/* LD_PRELOAD is unset; setenv preload directly! */
	curr = _getenv("LD_PRELOAD");
	if (!curr)
		return _setenv("LD_PRELOAD", preload, overwrite);

	n = _snprintf(buf, sizeof(buf), "%s:%s", preload, curr);
	if (n == -1)
		return -1;

	return _setenv("LD_PRELOAD", buf, overwrite);
}

static regex_t *re_ldso;
static regex_t *re_lib;

constructor void dso_init()
{
	static regex_t regex_ldso, regex_lib;
	const char *ldso = "^ld(64|-[[:alnum:]._-]+)?\\.so(\\.[[:digit:]]+)?$";
	const char *lib = "^lib[[:alnum:]+._-]+\\.so(\\.[[:alnum:]_-]+)*$";
	int ret;

	if (re_ldso)
		goto lib;

	ret = jim_regcomp(&regex_ldso, ldso, REG_EXTENDED);
	if (ret != 0) {
		jim_regex_perror("jim_regcomp", &regex_ldso, ret);
		return;
	}

	re_ldso = &regex_ldso;

lib:
	if (re_lib)
		return;

	ret = jim_regcomp(&regex_lib, lib, REG_EXTENDED);
	if (ret != 0) {
		jim_regex_perror("jim_regcomp", &regex_lib, ret);
		return;
	}

	re_lib = &regex_lib;
}

destructor void dso_fini()
{
	if (!re_lib)
		goto ldso;

	jim_regfree(re_lib);
	re_lib = NULL;

ldso:
	if (!re_ldso)
		return;

	jim_regfree(re_ldso);
	re_ldso = NULL;
}

hidden int __is_ldso(const char *path)
{
	regmatch_t match;
	int ret = 0;

	if (!re_ldso)
		return 0;

	ret = jim_regexec(re_ldso, path, 1, &match, 0);
	if (ret > REG_NOMATCH) {
		jim_regex_perror("jim_regexec", re_ldso, ret);
		return 0;
	}

	return !ret;
}

static int __is_lib(const char *path)
{
	regmatch_t match;
	int ret = 0;

	if (!re_lib)
		return 0;

	ret = jim_regexec(re_lib, path, 1, &match, 0);
	if (ret > REG_NOMATCH) {
		jim_regex_perror("jim_regexec", re_lib, ret);
		return 0;
	}

	return !ret;
}

static char *__path_strncat(char *dst, const char *src, size_t dstsiz)
{
	size_t len;

	if (!src)
		return __set_errno_and_perror(EINVAL, NULL);

	len = strlen(src);
	if (*dst)
		len++;
	if (len > (dstsiz-strlen(dst)-1)) /* NULL-terminated */
		return __set_errno_and_perror(ENOSPC, NULL);

	if (*dst && *src)
		_strncat(dst, ":", dstsiz);

	return _strncat(dst, src, dstsiz);
}

static int __is_in_path_callback(const char *path, void *user)
{
	const char *p = (const char *)user;

	return strneq(path, p, PATH_MAX);
}

static int __is_in_path(const char *pathname, const char *path)
{
	return __path_iterate(path, __is_in_path_callback, (void *)pathname);
}

static int __is_in_preload_callback(const char *path, void *user)
{
	const char *p = (const char *)user;

	return strneq(__basename(path), __basename(p), PATH_MAX);
}

static int __is_in_preload(const char *path, const char *preload)
{
	return __path_iterate(preload, __is_in_preload_callback, (void *)path);
}

static int __is_preloading_so(const char *so)
{
	const char *ld_preload;

	ld_preload = _getenv("LD_PRELOAD");
	if (!ld_preload)
		return 0;

	return __is_in_preload(so, ld_preload);
}

static int __preload_so(const char *so)
{
	int err;

	err = __is_preloading_so("libiamroot.so");
	if (err == -1)
		return -1;
	if (err == 1)
		return 1;

	err = __is_preloading_so(so);
	if (err == -1)
		return -1;

	return __setld_preload(so, 1);
}

/*
 * Stolen and hacked from musl (ldso/dynlink.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
/* The original function name is fixup_rpath() in the musl sources */
static int __variable_has_dynamic_string_tokens(const char *value)
{
	const char *s, *t;
	int n;

	if (!strchr(value, '$'))
		return 0;

	n = 0;
	s = value;
	while ((t = strchr(s, '$'))) {
		if (__strneq(t, "$ORIGIN") && __strneq(t, "${ORIGIN}"))
			return 1;
		if (__strneq(t, "$LIB") && __strneq(t, "${LIB}"))
			return 1;
		if (__strneq(t, "$PLATFORM") && __strneq(t, "${PLATFORM}"))
			return 1;
		s = t + 1;
		n++;
	}

	return n;
}

static ssize_t __elf_needed(const char *, char *, size_t);
static ssize_t __felf_needed(int, char *, size_t);
static ssize_t __felf_rpath(int, char *, size_t, off_t);
static ssize_t __felf_runpath(int, char *, size_t, off_t);
static int __felf_flags_1(int, uint32_t *);
static ssize_t __felf_deflib(int, char *, size_t, off_t);
static int __elf_so_context(const char *, char **, char **, char **, char **,
			    uint32_t *, char *, size_t, off_t);
static int __felf_so_context(int, char **, char **, char **, char **,
			     uint32_t *, char *, size_t, off_t);

static int __ldso_preload_needed(const char *, const char *, char *, size_t,
				 off_t);

struct __ldso_preload_needed_context {
	const char *deflib;
	char *buf;
	size_t bufsiz;
	off_t offset;
};

static int __ldso_preload_needed_callback(const char *needed, void *user)
{
	struct __ldso_preload_needed_context *ctx =
				  (struct __ldso_preload_needed_context *)user;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	/* FIXME: skipping */
	if (*needed == '/')
		return 0;

	siz = __path_access(needed, F_OK, ctx->deflib, buf, sizeof(buf));
	if (siz == -1 && errno == ENOENT)
		__warning("%s: needed library not found in library-path %s\n",
			  needed, ctx->deflib);
	if (siz == -1)
		return -1;

	/* Ignore none-libraries (i.e. linux-vdso.so.1, ld.so-ish...) */
	ret = __is_lib(__basename(buf));
	if (ret == 0 && !__is_ldso(__basename(buf)))
		__warning("%s: ignoring non-library!\n", __basename(buf));
	if (ret == 0)
		return 0;

	/* The shared object is already in the buffer */
	ret = __is_in_path(buf, &ctx->buf[ctx->offset]);
	if (ret == -1 || ret == 1)
		return 0;

	/* Add the needed shared objects to buf */
	return __ldso_preload_needed(buf, ctx->deflib, ctx->buf, ctx->bufsiz,
				     ctx->offset);
}

static int __ldso_preload_needed(const char *path, const char *deflib,
				 char *buf, size_t bufsiz, off_t offset)
{
	struct __ldso_preload_needed_context ctx;
	char needed[PATH_MAX];
	ssize_t siz;
	char *str;

	/*
	 * Add the shared object to path first to avoid circular dependencies,
	 * if a needed shared object needs this one.
	 */
	str = __path_strncat(buf+offset, path, bufsiz-offset);
	if (!str)
		return -1;
	/* Add the needed shared objects to path then */
	siz = __elf_needed(path, needed, sizeof(needed));
	if (siz == -1)
		return -1;
	ctx.deflib = deflib;
	ctx.buf = buf;
	ctx.bufsiz = bufsiz;
	ctx.offset = offset;
	return __path_iterate(needed, __ldso_preload_needed_callback, &ctx);
}

static int __ldso_preload_executable(const char *path, const char *deflib,
				     char *buf, size_t bufsiz, off_t offset)
{
	struct __ldso_preload_needed_context ctx;
	char needed[PATH_MAX];
	ssize_t siz;

	/* Add the needed shared objects to path only */
	siz = __elf_needed(path, needed, sizeof(needed));
	if (siz == -1)
		return -1;
	ctx.deflib = deflib;
	ctx.buf = buf;
	ctx.bufsiz = bufsiz;
	ctx.offset = offset;
	return __path_iterate(needed, __ldso_preload_needed_callback, &ctx);
}

static int __secure_execution_mode();

/*
 * Note: This resolves the shared object as described by ld-linux.so(8).
*/
static ssize_t __ldso_access(const char *path,
			     int mode,
			     const char *exec_rpath,
			     const char *exec_runpath,
			     const char *rpath,
			     const char *ld_library_path,
			     const char *runpath,
			     const char *deflib,
			     uint32_t flags_1,
			     char *buf,
			     size_t bufsiz)
{
	ssize_t ret = -1;

	/*
	 * According to ld-linux.so(8)
	 *
	 * If a shared object dependency does not contain a slash, then it is
	 * searched for in the following order:
	 */
	if (strchr(path, '/')) {
		_strncpy(buf, path, bufsiz);

		return strnlen(buf, bufsiz);
	}

	/* Search for NAME in several places. */

	/*
	 * (1)  Using the directories specified in the DT_RPATH dynamic section
	 * attribute of the binary if present and DT_RUNPATH attribute does not
	 * exist. Use of DT_RPATH is deprecated.
	 */
	if (!runpath) {
		/*
		 * According to glibc (elf/dl-load.c)
		 *
		 * First try the DT_RPATH of the dependent object that caused
		 * NAME to be loaded. Then that object's dependent, and on up.
		 */
		__info("%s: trying accessing from shared object DT_RPATH %s...\n",
		       path, rpath);
		ret = __path_access(path, mode, rpath, buf, bufsiz);
		if (ret == -1 && errno != ENOENT)
			return -1;

		if (ret > 0)
			return ret;

		/*
		 * If dynamically linked, try the DT_RPATH of the executable
		 * itself.
		 */
		__info("%s: trying accessing from executable DT_RPATH %s...\n",
		       path, exec_rpath);
		ret = __path_access(path, mode, exec_rpath, buf, bufsiz);
		if (ret == -1 && errno != ENOENT)
			return -1;

		if (ret > 0)
			return ret;

		/*
		 * Also try DT_RUNPATH in the executable for LD_AUDIT dlopen
		 * call.
		 */
		/* TODO: This is not usefull, at least for now. */
		(void)exec_runpath;
	}

	/*
	 * (2)  Using the environment variable LD_LIBRARY_PATH, unless the
	 * executable is being run in secure-execution mode (see below), in
	 * which case this variable is ignored.
	 */
	if (ld_library_path && !__secure_execution_mode()) {
		__info("%s: trying accessing from LD_LIBRARY_PATH %s...\n",
		       path, ld_library_path);
		ret = __path_access(path, mode, ld_library_path, buf, bufsiz);
		if (ret == -1 && errno != ENOENT)
			return -1;

		if (ret > 0)
			return ret;
	}

	/*
	 * (3)  Using the directories specified in the DT_RUNPATH dynamic
	 * section attribute of the binary if present. Such directories are
	 * searched only to find those objects required by DT_NEEDED (direct
	 * dependencies) entries and do not apply to those objects' children,
	 * which must themselves have their own DT_RUNPATH entries. This is
	 * unlike DT_RPATH, which is applied to searches for all children in
	 * the dependency tree.
	 */
	if (runpath) {
		__info("%s: trying accessing from shared object DT_RUNPATH %s...\n",
		       path, runpath);
		ret = __path_access(path, mode, runpath, buf, bufsiz);
		if (ret == -1 && errno != ENOENT)
			return -1;

		if (ret > 0)
			return ret;
	}

	/*
	 * (4)  From the cache file /etc/ld.so.cache, which contains a compiled
	 * list of candidate shared objects previously found in the augmented
	 * library path. If, however, the binary was linked with the -z
	 * nodeflib linker option, shared objects in the default paths are
	 * skipped. Shared objects installed in hardware capability directories
	 * are preferred to other shared objects.
	 */
	__info("%s: trying accessing from cache...\n", path);
	ret = __ldso_cache(path, buf, bufsiz);
	if (ret == -1 && errno != ENOENT)
		return -1;

	if (ret > 0)
		return ret;

	/*
	 * (5)  In the default path /lib, and then /usr/lib. (On some 64-bit
	 * architectures, the default paths for 64-bit shared objects are
	 * /lib64, and then /usr/lib64.) If the binary was linked with the
	 * -z nodeflib linker option, this step is skipped.
	 */
	if (!(flags_1 & DF_1_NODEFLIB) && deflib) {
		__info("%s: trying accessing from default library path %s...\n",
		       path, deflib);
		ret = __path_access(path, mode, deflib, buf, bufsiz);
		if (ret == -1 && errno != ENOENT)
			return -1;

		if (ret > 0)
			return ret;
	}

	return __set_errno(ENOENT, -1);
}

#if defined(__linux__) || defined(__FreeBSD__)
static int __dl_is_opened(const char *path)
{
	struct link_map *dso;
	void *handle;
	int err;

	handle = next_dlopen(NULL, RTLD_NOW);
	if (!handle)
		return -1;

	err = dlinfo(handle, RTLD_DI_LINKMAP, &dso);
	if (err == -1)
		return -1;

	while (dso) {
		if (streq(path, dso->l_name))
			return 1;

		dso = dso->l_next;
	}

	return 0;
}
#else
static int __dl_is_opened_callback(struct dl_phdr_info *info, size_t size,
				   void *data)
{
	const char *path = (const char *)data;
	(void)size;

	return streq(__basename(path), __basename(info->dlpi_name));
}

static int __dl_is_opened(const char *path)
{
	return dl_iterate_phdr(__dl_is_opened_callback, (void *)path);
}
#endif

static int __ld_open_needed(const char *, int, const char *);

struct __ld_needed_context {
	const char *rpath;
	const char *ld_library_path;
	const char *runpath;
	const char *deflib;
	uint32_t flags_1;
	int (*callback)(const char *, const char *, const char *,
			const char *, const char *, uint32_t, void *);
	void *user;
};

static int __ld_needed_callback(const char *needed, void *user)
{
	struct __ld_needed_context *ctx = (struct __ld_needed_context *)user;

	if (!ctx->callback)
		return __set_errno_and_perror(EINVAL, -1);

	return ctx->callback(needed, ctx->rpath, ctx->ld_library_path,
			     ctx->runpath, ctx->deflib, ctx->flags_1,
			     ctx->user);
}

static int __fld_needed(int fd,
			const char *rpath,
			const char *ld_library_path,
			const char *runpath,
			const char *deflib,
			uint32_t flags_1,
			int (*callback)(const char *, const char *,
					const char *, const char *,
					const char *, uint32_t, void *),
		       void *user)
{
	struct __ld_needed_context ctx;
	char needed[PATH_MAX];
	ssize_t siz;

	/* Iterate over the DT_NEEDED shared objects */
	siz = __felf_needed(fd, needed, sizeof(needed));
	if (siz == -1)
		return -1;
	ctx.rpath = rpath;
	ctx.ld_library_path = ld_library_path;
	ctx.runpath = runpath;
	ctx.deflib = deflib;
	ctx.flags_1 = flags_1;
	ctx.callback = callback;
	ctx.user = user;
	return __path_iterate(needed, __ld_needed_callback, &ctx);
}

#ifndef __NetBSD__
static int __ld_needed(const char *path,
		       const char *rpath,
		       const char *ld_library_path,
		       const char *runpath,
		       const char *deflib,
		       uint32_t flags_1,
		       int (*callback)(const char *, const char *,
				       const char *, const char *,
				       const char *, uint32_t, void *),
		       void *user)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fld_needed(fd, rpath, ld_library_path, runpath, deflib,
			   flags_1, callback, user);

	__close(fd);

	return ret;
}
#endif

struct __ld_open_needed_context {
	int flags;
	const char *needed_by;
};

static int __ld_open_needed_callback(const char *needed,
				     const char *rpath,
				     const char *ld_library_path,
				     const char *runpath,
				     const char *deflib,
				     uint32_t flags_1,
				     void *user)
{
	struct __ld_open_needed_context *ctx =
				       (struct __ld_open_needed_context *)user;
	char buf[PATH_MAX];
	void *handle;
	ssize_t siz;
	int ret;

	/* Look for the shared object */
	siz = __ldso_access(needed, F_OK, NULL, NULL, rpath, ld_library_path,
			    runpath, deflib, flags_1, buf, sizeof(buf));
	if (siz == -1)
		return -1;

	/* Open the needed shared objects first */
	ret = __ld_open_needed(buf, ctx->flags, ctx->needed_by);
	if (ret == -1)
		return -1;

	/* Open the shared object then */
	handle = next_dlopen(buf, ctx->flags);
	if (!handle)
		return -1;

	__notice("%s: dlopen'ed needed shared object \"%s\"\n", ctx->needed_by,
		 buf);
	return 0;
}

static int __fld_open_needed(int fd, int flags, const char *needed_by)
{
	char *deflib, *ld_library_path, *rpath, *runpath;
	struct __ld_open_needed_context ctx;
	char tmp[PATH_MAX];
	uint32_t flags_1;
	ssize_t siz;
	int ret;

	siz = fpath(fd, tmp, sizeof(tmp));
	if (siz == -1)
		return -1;

	/* The shared object is already opened */
	ret = __dl_is_opened(tmp);
	if (ret == -1 || ret == 1)
		return ret;

	/* Get the ELF shared object context */
	ret = __felf_so_context(fd, &rpath, &ld_library_path, &runpath,
				&deflib, &flags_1, tmp, sizeof(tmp), 0);
	if (ret == -1)
		return -1;

	/* Open the needed shared objects */
	ctx.flags = flags;
	ctx.needed_by = needed_by;
	return __fld_needed(fd, rpath, ld_library_path, runpath, deflib,
			    flags_1, __ld_open_needed_callback, &ctx);
}

static int __ld_open_needed(const char *path, int flags, const char *needed_by)
{
	int fd, ret;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fld_open_needed(fd, flags, needed_by);

	__close(fd);

	return ret;
}

hidden int __dlopen_needed(const char *path, int flags)
{
	return __ld_open_needed(path, flags, __execfn());
}

static int __ld_ldso_abi(const char *path, char ldso[NAME_MAX], int *abi)
{
	const char *name;
	int n;

	name = __basename(path);
	if (!name)
		return __set_errno(ENOTSUP, -1);

	/* Generic dynamic loader (ld.so) */
	if (streq(name, "ld.so")) {
		*ldso = 0;
		if (abi)
			*abi = -1;
		return 0;
	}

	/* Linux dynamic loader (ld.so.<abi>) */
	n = sscanf(name, "ld.so.%i", abi);
	if (n == 1) {
		*ldso = 0;
		return 0;
	}

	/* Linux 64 dynamic loader (ld64.so.<abi>) */
	n = sscanf(name, "ld64.so.%i", abi);
	if (n == 1) {
		*ldso = 0;
		return 0;
	}

	/* NetBSD dynamic loader (ld.<object>_so) */
	n = sscanf(name, "ld.%" __xstr(NAME_MAX) "[^_]_so", ldso);
	if (n == 1) {
		if (abi)
			*abi = -1;
		return 0;
	}

	/* Linux or FreeBSD dynamic loader (ld-<ldso>.so.<abi>) */
	n = sscanf(name, "ld-%" __xstr(NAME_MAX) "[^.].so.%i", ldso, abi);
	if (n == 2)
		return 0;

	return __set_errno(ENOTSUP, -1);
}

static int __ld_linux_version(const char *path, int *major, int *minor)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int n;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_FOLLOW);
	if (siz == -1)
		return -1;

	n = sscanf(__basename(buf), "ld-%i.%i.so", major, minor);
	if (n < 2)
		return __set_errno(ENOTSUP, -1);

	return 0;
}

static int __ld_linux_has_inhibit_cache_option(const char *path)
{
	const int errno_save = errno;
	int ret, maj = 0, min = 0;

	ret = __ld_linux_version(path, &maj, &min);
	if ((ret == -1) && (errno != ENOTSUP))
		return -1;

	if (ret == -1)
		return __set_errno(errno_save, 1);

	/* --inhibit-cache is supported since glibc 2.16 */
	ret = (maj > 2) || ((maj == 2) && (min >= 16));
	return __set_errno(errno_save, ret);
}

static int __ld_linux_has_argv0_option(const char *path)
{
	const int errno_save = errno;
	int ret, maj = 0, min = 0;

	ret = __ld_linux_version(path, &maj, &min);
	if ((ret == -1) && (errno != ENOTSUP))
		return -1;

	if (ret == -1)
		return __set_errno(errno_save, 1);

	/* --argv0 is supported since glibc 2.33 */
	ret = (maj > 2) || ((maj == 2) && (min >= 33));
	return __set_errno(errno_save, ret);
}

static int __ld_linux_has_preload_option(const char *path)
{
	const int errno_save = errno;
	int ret, maj = 0, min = 0;

	ret = __ld_linux_version(path, &maj, &min);
	if ((ret == -1) && (errno != ENOTSUP))
		return -1;

	if (ret == -1)
		return __set_errno(errno_save, 1);

	/* --preload is supported since glibc 2.30 */
	ret = (maj > 2) || ((maj == 2) && (min >= 30));
	return __set_errno(errno_save, ret);
}

static int __elf32_swap(Elf32_Ehdr *ehdr)
{
	union { int i; char c[4]; } u = { 1 };
	return (u.c[3] + 1) != ehdr->e_ident[EI_DATA];
}

static uint16_t __elf32_half(Elf32_Ehdr *ehdr, uint16_t value)
{
	if (!__elf32_swap(ehdr))
		return value;

	return __bswap_16__(value);
}

static uint32_t __elf32_word(Elf32_Ehdr *ehdr, uint32_t value)
{
	if (!__elf32_swap(ehdr))
		return value;

	return __bswap_32__(value);
}

static uint32_t __elf32_address(Elf32_Ehdr *ehdr, uint32_t value)
{
	if (!__elf32_swap(ehdr))
		return value;

	return __bswap_32__(value);
}

static uint32_t __elf32_offset(Elf32_Ehdr *ehdr, uint32_t value)
{
	if (!__elf32_swap(ehdr))
		return value;

	return __bswap_32__(value);
}

static int __elf32_phdr(int fd, Elf32_Ehdr *ehdr, Elf32_Phdr *phdr,
			off_t offset)
{
	ssize_t siz;

	siz = pread(fd, phdr, sizeof(*phdr), offset);
	if (siz == -1)
		return -1;
	else if ((size_t)siz < sizeof(*phdr))
		return __set_errno_and_perror(EIO, -1);

	if (!__elf32_swap(ehdr))
		return 0;

	phdr->p_type = __bswap_32__(phdr->p_type);
	phdr->p_offset = __bswap_32__(phdr->p_offset);
	phdr->p_vaddr = __bswap_32__(phdr->p_vaddr);
	phdr->p_paddr = __bswap_32__(phdr->p_paddr);
	phdr->p_filesz = __bswap_32__(phdr->p_filesz);
	phdr->p_memsz = __bswap_32__(phdr->p_memsz);
	phdr->p_flags = __bswap_32__(phdr->p_flags);
	phdr->p_align = __bswap_32__(phdr->p_align);

	return 0;
}

static int __elf32_shdr(int fd, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr,
			off_t offset)
{
	ssize_t siz;

	siz = pread(fd, shdr, sizeof(*shdr), offset);
	if (siz == -1)
		return -1;
	else if ((size_t)siz < sizeof(*shdr))
		return __set_errno_and_perror(EIO, -1);

	if (!__elf32_swap(ehdr))
		return 0;

	shdr->sh_name = __bswap_32__(shdr->sh_name);
	shdr->sh_type = __bswap_32__(shdr->sh_type);
	shdr->sh_flags = __bswap_32__(shdr->sh_flags);
	shdr->sh_addr = __bswap_32__(shdr->sh_addr);
	shdr->sh_offset = __bswap_32__(shdr->sh_offset);
	shdr->sh_size = __bswap_32__(shdr->sh_size);
	shdr->sh_link = __bswap_32__(shdr->sh_link);
	shdr->sh_addralign = __bswap_32__(shdr->sh_addralign);
	shdr->sh_entsize = __bswap_32__(shdr->sh_entsize);

	return 0;
}

static int __elf32_dyn(int fd, Elf32_Ehdr *ehdr, Elf32_Dyn *dyn, size_t dynsiz,
		       off_t offset)
{
	ssize_t siz;

	siz = pread(fd, dyn, dynsiz, offset);
	if (siz == -1)
		return -1;
	else if ((size_t)siz < dynsiz)
		return __set_errno_and_perror(EIO, -1);

	if (!__elf32_swap(ehdr))
		return 0;

	dyn->d_tag = __bswap_32__(dyn->d_tag);
	if (dyn->d_tag != DT_NEEDED && dyn->d_tag != DT_SONAME &&
	    dyn->d_tag != DT_RPATH && dyn->d_tag != DT_RUNPATH)
		dyn->d_un.d_val = __bswap_32__(dyn->d_un.d_val);
	else
		dyn->d_un.d_ptr = __bswap_32__(dyn->d_un.d_ptr);

	return 0;
}

static int __elf64_swap(Elf64_Ehdr *ehdr)
{
	union { int i; char c[4]; } u = { 1 };
	return (u.c[3] + 1) != ehdr->e_ident[EI_DATA];
}

static uint16_t __elf64_half(Elf64_Ehdr *ehdr, uint16_t value)
{
	if (!__elf64_swap(ehdr))
		return value;

	return __bswap_16__(value);
}

static uint32_t __elf64_word(Elf64_Ehdr *ehdr, uint32_t value)
{
	if (!__elf64_swap(ehdr))
		return value;

	return __bswap_32__(value);
}

static uint64_t __elf64_address(Elf64_Ehdr *ehdr, uint64_t value)
{
	if (!__elf64_swap(ehdr))
		return value;

	return __bswap_64__(value);
}

static uint64_t __elf64_offset(Elf64_Ehdr *ehdr, uint64_t value)
{
	if (!__elf64_swap(ehdr))
		return value;

	return __bswap_64__(value);
}

static int __elf64_phdr(int fd, Elf64_Ehdr *ehdr, Elf64_Phdr *phdr,
			off_t offset)
{
	ssize_t siz;

	siz = pread(fd, phdr, sizeof(*phdr), offset);
	if (siz == -1)
		return -1;
	else if ((size_t)siz < sizeof(*phdr))
		return __set_errno_and_perror(EIO, -1);

	if (!__elf64_swap(ehdr))
		return 0;

	phdr->p_type = __bswap_32__(phdr->p_type);
	phdr->p_flags = __bswap_32__(phdr->p_flags);
	phdr->p_offset = __bswap_64__(phdr->p_offset);
	phdr->p_vaddr = __bswap_64__(phdr->p_vaddr);
	phdr->p_paddr = __bswap_64__(phdr->p_paddr);
	phdr->p_filesz = __bswap_64__(phdr->p_filesz);
	phdr->p_memsz = __bswap_64__(phdr->p_memsz);
	phdr->p_align = __bswap_64__(phdr->p_align);

	return 0;
}

static int __elf64_shdr(int fd, Elf64_Ehdr *ehdr, Elf64_Shdr *shdr,
			off_t offset)
{
	ssize_t siz;

	siz = pread(fd, shdr, sizeof(*shdr), offset);
	if (siz == -1)
		return -1;
	else if ((size_t)siz < sizeof(*shdr))
		return __set_errno_and_perror(EIO, -1);

	if (!__elf64_swap(ehdr))
		return 0;

	shdr->sh_name = __bswap_32__(shdr->sh_name);
	shdr->sh_type = __bswap_32__(shdr->sh_type);
	shdr->sh_flags = __bswap_64__(shdr->sh_flags);
	shdr->sh_addr = __bswap_64__(shdr->sh_addr);
	shdr->sh_offset = __bswap_64__(shdr->sh_offset);
	shdr->sh_size = __bswap_64__(shdr->sh_size);
	shdr->sh_link = __bswap_32__(shdr->sh_link);
	shdr->sh_info = __bswap_32__(shdr->sh_info);
	shdr->sh_addralign = __bswap_64__(shdr->sh_addralign);
	shdr->sh_entsize = __bswap_64__(shdr->sh_entsize);

	return 0;
}

static int __elf64_dyn(int fd, Elf64_Ehdr *ehdr, Elf64_Dyn *dyn, size_t dynsiz,
		       off_t offset)
{
	ssize_t siz;

	siz = pread(fd, dyn, dynsiz, offset);
	if (siz == -1)
		return -1;
	else if ((size_t)siz < dynsiz)
		return __set_errno_and_perror(EIO, -1);

	if (!__elf64_swap(ehdr))
		return 0;

	dyn->d_tag = __bswap_64__(dyn->d_tag);
	if (dyn->d_tag != DT_NEEDED && dyn->d_tag != DT_SONAME &&
	    dyn->d_tag != DT_RPATH && dyn->d_tag != DT_RUNPATH)
		dyn->d_un.d_val = __bswap_64__(dyn->d_un.d_val);
	else
		dyn->d_un.d_ptr = __bswap_64__(dyn->d_un.d_ptr);

	return 0;
}

static int __elf_ehdr(int fd, Elf64_Ehdr *ehdr)
{
	ssize_t siz;

	siz = read(fd, ehdr, sizeof(*ehdr));
	if (siz == -1)
		return -1;
	else if ((size_t)siz < sizeof(*ehdr))
		return __set_errno_and_perror(EIO, -1);

	/* Not an ELF */
	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
		return __set_errno(ENOEXEC, -1);

	/* It is a 32-bit ELF */
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
		Elf32_Ehdr *ehdr32 = (Elf32_Ehdr *)ehdr;

		ehdr32->e_type = __elf32_half(ehdr32, ehdr32->e_type);
		ehdr32->e_machine = __elf32_half(ehdr32, ehdr32->e_machine);
		ehdr32->e_version = __elf32_word(ehdr32, ehdr32->e_version);
		ehdr32->e_entry = __elf32_address(ehdr32, ehdr32->e_entry);
		ehdr32->e_phoff = __elf32_offset(ehdr32, ehdr32->e_phoff);
		ehdr32->e_shoff = __elf32_offset(ehdr32, ehdr32->e_shoff);
		ehdr32->e_flags = __elf32_word(ehdr32, ehdr32->e_flags);
		ehdr32->e_ehsize = __elf32_half(ehdr32, ehdr32->e_ehsize);
		ehdr32->e_phentsize = __elf32_half(ehdr32, ehdr32->e_phentsize);
		ehdr32->e_phnum = __elf32_half(ehdr32, ehdr32->e_phnum);
		ehdr32->e_shentsize = __elf32_half(ehdr32, ehdr32->e_phentsize);
		ehdr32->e_shnum = __elf32_half(ehdr32, ehdr32->e_shnum);
		ehdr32->e_shstrndx = __elf32_half(ehdr32, ehdr32->e_shstrndx);

		return 0;
	}

	/* It is a 64-bit ELF */
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64) {
		Elf64_Ehdr *ehdr64 = ehdr;

		ehdr64->e_type = __elf64_half(ehdr64, ehdr64->e_type);
		ehdr64->e_machine = __elf64_half(ehdr64, ehdr64->e_machine);
		ehdr64->e_version = __elf64_word(ehdr64, ehdr64->e_version);
		ehdr64->e_entry = __elf64_address(ehdr64, ehdr64->e_entry);
		ehdr64->e_phoff = __elf64_offset(ehdr64, ehdr64->e_phoff);
		ehdr64->e_shoff = __elf64_offset(ehdr64, ehdr64->e_shoff);
		ehdr64->e_flags = __elf64_word(ehdr64, ehdr64->e_flags);
		ehdr64->e_ehsize = __elf64_half(ehdr64, ehdr64->e_ehsize);
		ehdr64->e_phentsize = __elf64_half(ehdr64, ehdr64->e_phentsize);
		ehdr64->e_phnum = __elf64_half(ehdr64, ehdr64->e_phnum);
		ehdr64->e_shentsize = __elf64_half(ehdr64, ehdr64->e_phentsize);
		ehdr64->e_shnum = __elf64_half(ehdr64, ehdr64->e_shnum);
		ehdr64->e_shstrndx = __elf64_half(ehdr64, ehdr64->e_shstrndx);

		return 0;
	}

	/* It is an invalid ELF */
	return __set_errno(ENOEXEC, -1);
}

static int __felf_header(int fd, Elf64_Ehdr *ehdr)
{
	int err;

	if (fd < 0 || !ehdr)
		return __set_errno_and_perror(EINVAL, -1);

	err = lseek(fd, 0, SEEK_SET);
	if (err)
		return -1;

	return __elf_ehdr(fd, ehdr);
}

static ssize_t __elf_interp32(int fd, Elf32_Ehdr *ehdr, char *buf,
			      size_t bufsiz)
{
	ssize_t ret = -1;
	int i, num;
	off_t off;

	/* Look for the .interp section */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf32_Phdr phdr;
		int err;

		err = __elf32_phdr(fd, ehdr, &phdr, off);
		if (err == -1)
			goto exit;

		off += sizeof(phdr);

		/* Not the .interp section */
		if (phdr.p_type != PT_INTERP)
			continue;

		if (bufsiz < phdr.p_filesz) {
			ret = __set_errno_and_perror(EIO, -1);
			goto exit;
		}

		/* copy the NULL-terminated string from the .interp section */
		ret = pread(fd, buf, phdr.p_filesz, phdr.p_offset);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < phdr.p_filesz) {
			ret = __set_errno_and_perror(EIO, -1);
			goto exit;
		}

		errno = 0;
		goto exit;
	}

	ret = __set_errno(ENOEXEC, -1);

exit:
	return ret;
}

static ssize_t __elf_interp64(int fd, Elf64_Ehdr *ehdr, char *buf,
			     size_t bufsiz)
{
	ssize_t ret = -1;
	int i, num;
	off_t off;

	/* Look for the .interp section */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf64_Phdr phdr;
		int err;

		err = __elf64_phdr(fd, ehdr, &phdr, off);
		if (err == -1)
			goto exit;

		off += sizeof(phdr);

		/* Not the .interp section */
		if (phdr.p_type != PT_INTERP)
			continue;

		if (bufsiz < phdr.p_filesz) {
			ret = __set_errno_and_perror(EIO, -1);
			goto exit;
		}

		/* copy the NULL-terminated string from the .interp section */
		ret = pread(fd, buf, phdr.p_filesz, phdr.p_offset);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < phdr.p_filesz) {
			ret = __set_errno_and_perror(EIO, -1);
			goto exit;
		}

		errno = 0;
		goto exit;
	}

	ret = __set_errno(ENOEXEC, -1);

exit:
	return ret;
}

static int __elf_iterate_ehdr32(int fd, Elf32_Ehdr *ehdr, int d_tag,
		     int (*callback)(const void *, size_t, void *), void *data)
{
	const int errno_save = errno;
	size_t strtab_siz = 0;
	off_t strtab_off = 0;
	Elf32_Dyn dyn[1024];
	ssize_t siz = -1;
	int ret = -1;
	int i, num;
	off_t off;

	if (!callback)
		ret = __set_errno_and_perror(EINVAL, -1);

	/* Look for the .shstrtab section */
	off = ehdr->e_shoff;
	num = ehdr->e_shnum;
	for (i = 0; i < num; i++) {
		Elf32_Shdr shdr;
		int err;

		err = __elf32_shdr(fd, ehdr, &shdr, off);
		if (err == -1)
			goto exit;

		off += sizeof(shdr);

		/* Not the .shstrtab section */
		if (shdr.sh_type != SHT_STRTAB)
			continue;

		strtab_siz = shdr.sh_size;
		strtab_off = shdr.sh_offset;
		break;
	}

	/* No .shstrtab section */
	if (!strtab_off)
		goto exit;

	/* Look for the dynamic entry in the .dynamic segment */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf32_Phdr phdr;
		unsigned int j;
		size_t str_siz;
		off_t str_off;
		int err;

		err = __elf32_phdr(fd, ehdr, &phdr, off);
		if (err == -1)
			goto exit;

		off += sizeof(phdr);

		/* Not the .dynamic segment */
		if (phdr.p_type != PT_DYNAMIC)
			continue;

		if (sizeof(dyn) < phdr.p_filesz) {
			ret = __set_errno_and_perror(EIO, -1);
			goto exit;
		}

		/* copy the .dynamic segment */
		err = __elf32_dyn(fd, ehdr, dyn, phdr.p_filesz, phdr.p_offset);
		if (err == -1)
			goto exit;

		/* loop for the dynamic entries */
		for (j = 0; j < phdr.p_filesz / sizeof(dyn[0]); j++) {
			char buf[BUFSIZ];
			size_t size;

			if ((int)dyn[j].d_tag != d_tag)
				continue;

			/*
			 * dymamic entry is an integer
			 */
			if (d_tag != DT_NEEDED && d_tag != DT_SONAME &&
			    d_tag != DT_RPATH && d_tag != DT_RUNPATH) {
				ret = callback(&dyn[j].d_un.d_val,
					       sizeof(dyn[j].d_un.d_val),
					       data);
				if (ret != 0)
					break;

				continue;
			}

			/*
			 * dynamic entry is a string
			 *
			 * copy the NULL-terminated string from the .strtab
			 * table
			 */
			str_off = strtab_off + dyn[j].d_un.d_ptr;
			str_siz = strtab_siz - dyn[j].d_un.d_ptr;
			size = __min(str_siz, sizeof(buf));
			siz = pread(fd, buf, size, str_off);
			if (siz == -1) {
				goto exit;
			} else if ((size_t)siz < size) {
				ret = __set_errno_and_perror(EIO, -1);
				goto exit;
			}

			ret = callback(buf, size, data);
			if (ret != 0)
				break;
		}
	}

	ret = 0;

exit:
	if (ret != -1)
		errno = errno_save;

	return ret;
}

static int __elf_iterate_ehdr64(int fd, Elf64_Ehdr *ehdr, int d_tag,
		     int (*callback)(const void *, size_t, void *), void *data)
{
	const int errno_save = errno;
	size_t strtab_siz = 0;
	off_t strtab_off = 0;
	Elf64_Dyn dyn[1024];
	ssize_t siz = -1;
	int ret = -1;
	int i, num;
	off_t off;

	if (!callback)
		ret = __set_errno_and_perror(EINVAL, -1);

	/* Look for the .shstrtab section */
	off = ehdr->e_shoff;
	num = ehdr->e_shnum;
	for (i = 0; i < num; i++) {
		Elf64_Shdr shdr;
		int err;

		err = __elf64_shdr(fd, ehdr, &shdr, off);
		if (err == -1)
			goto exit;

		off += sizeof(shdr);

		/* Not the .shstrtab section */
		if (shdr.sh_type != SHT_STRTAB)
			continue;

		strtab_siz = shdr.sh_size;
		strtab_off = shdr.sh_offset;
		break;
	}

	/* No .shstrtab section */
	if (!strtab_off)
		goto exit;

	/* Look for the dynamic entry in the .dynamic segment */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf64_Phdr phdr;
		unsigned int j;
		size_t str_siz;
		off_t str_off;
		int err;

		err = __elf64_phdr(fd, ehdr, &phdr, off);
		if (err == -1)
			goto exit;

		off += sizeof(phdr);

		/* Not the .dynamic segment */
		if (phdr.p_type != PT_DYNAMIC)
			continue;

		if (sizeof(dyn) < phdr.p_filesz) {
			ret = __set_errno_and_perror(EIO, -1);
			goto exit;
		}

		/* copy the .dynamic segment */
		err = __elf64_dyn(fd, ehdr, dyn, phdr.p_filesz, phdr.p_offset);
		if (err == -1)
			goto exit;

		/* loop for the dynamic entries */
		for (j = 0; j < phdr.p_filesz / sizeof(dyn[0]); j++) {
			char buf[BUFSIZ];
			size_t size;

			if ((int)dyn[j].d_tag != d_tag)
				continue;

			/*
			 * dymamic entry is an integer
			 */
			if (d_tag != DT_NEEDED && d_tag != DT_SONAME &&
			    d_tag != DT_RPATH && d_tag != DT_RUNPATH) {
				ret = callback(&dyn[j].d_un.d_val,
					       sizeof(dyn[j].d_un.d_val),
					       data);
				if (ret != 0)
					break;

				continue;
			}

			/*
			 * dynamic entry is a string
			 *
			 * copy the NULL-terminated string from the .strtab
			 * table
			 */
			str_off = strtab_off + dyn[j].d_un.d_ptr;
			str_siz = strtab_siz - dyn[j].d_un.d_ptr;
			size = __min(str_siz, sizeof(buf));
			siz = pread(fd, buf, size, str_off);
			if (siz == -1) {
				goto exit;
			} else if ((size_t)siz < size) {
				ret = __set_errno_and_perror(EIO, -1);
				goto exit;
			}

			ret = callback(buf, size, data);
			if (ret != 0)
				break;
		}
	}

	ret = 0;

exit:
	if (ret != -1)
		errno = errno_save;

	return ret;
}

static int __elf_iterate_shared_object(int fd, int d_tag,
		     int (*callback)(const void *, size_t, void *), void *data)
{
	Elf64_Ehdr ehdr;
	int err;

	err = lseek(fd, 0, SEEK_SET);
	if (err)
		return -1;

	err = __elf_ehdr(fd, &ehdr);
	if (err == -1)
		return -1;

	/* Not a linked program or shared object */
	if ((ehdr.e_type != ET_EXEC) && (ehdr.e_type != ET_DYN))
		return __set_errno(ENOEXEC, -1);

	/* It is a 32-bit ELF */
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		return __elf_iterate_ehdr32(fd, (Elf32_Ehdr *)&ehdr, d_tag,
					    callback, data);
	/* It is a 64-bit ELF */
	else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		return __elf_iterate_ehdr64(fd, (Elf64_Ehdr *)&ehdr, d_tag,
					    callback, data);

	/* It is an invalid ELF */
	return __set_errno(ENOEXEC, -1);
}

static ssize_t __felf_interp(int fd, char *buf, size_t bufsiz)
{
	Elf64_Ehdr ehdr;
	int err;

	/* Get the ELF header */
	err = __felf_header(fd, &ehdr);
	if (err == -1)
		return -1;

	/* It is a 32-bit ELF */
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		return __elf_interp32(fd, (Elf32_Ehdr *)&ehdr, buf, bufsiz);
	/* It is a 64-bit ELF */
	else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		return __elf_interp64(fd, &ehdr, buf, bufsiz);

	/* It is an invalid ELF */
	return __set_errno(ENOEXEC, -1);
}

static ssize_t __elf_interp(const char *path, char *buf, size_t bufsiz)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __felf_interp(fd, buf, bufsiz);

	__close(fd);

	return ret;
}

static void __env_sanitize(char *name, int upper)
{
	char *c;

	c = name;
	while (*c) {
		if (*c == '-')
			*c = '_';
		else if (upper)
			*c = toupper(*c);
		else
			*c = tolower(*c);
		c++;
	}
}

static ssize_t __is_linux_ldso(const char *ldso)
{
	return __strneq(ldso, "linux");
}

static ssize_t __is_musl_ldso(const char *ldso)
{
	return __strneq(ldso, "musl");
}

static ssize_t __is_elf_ldso(const char *ldso)
{
	return streq(ldso, "elf");
}

static int __is_32_bits(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is a 32-bit ELF */
	return ehdr && (ehdr->e_ident[EI_CLASS] == ELFCLASS32);
}

static int __is_64_bits(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is a 64-bit ELF */
	return ehdr && (ehdr->e_ident[EI_CLASS] == ELFCLASS64);
}

static int __is_x86(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is an x86 ELF */
	return ehdr && (ehdr->e_machine == EM_386);
}

static int __is_x86_64(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is an x86-64 ELF */
	return ehdr && (ehdr->e_machine == EM_X86_64);
}

static int __is_arm(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EF_ARM_ABI_FLOAT_SOFT
	/* It is an ARM ELF */
	return ehdr && (ehdr->e_machine == EM_ARM) &&
	       (((Elf32_Ehdr *)ehdr)->e_flags & EF_ARM_ABI_FLOAT_SOFT);
#elif defined EF_ARM_SOFT_FLOAT
	/* It is an ARM ELF */
	return ehdr && (ehdr->e_machine == EM_ARM) &&
	       (((Elf32_Ehdr *)ehdr)->e_flags & EF_ARM_SOFT_FLOAT);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_armhf(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EF_ARM_ABI_FLOAT_SOFT
	/* It is an ARMHF ELF */
	return ehdr && (ehdr->e_machine == EM_ARM) &&
	       (((Elf32_Ehdr *)ehdr)->e_flags & EF_ARM_ABI_FLOAT_HARD);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_aarch64(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EM_AARCH64
	/* It is an AArch64 LSB ELF */
	return ehdr && (ehdr->e_machine == EM_AARCH64) &&
	       (ehdr->e_ident[EI_DATA] == ELFDATA2LSB);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_aarch64_be(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EM_AARCH64
	/* It is an AArch64 MSB ELF */
	return ehdr && (ehdr->e_machine == EM_AARCH64) &&
	       (ehdr->e_ident[EI_DATA] == ELFDATA2MSB);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_riscv(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EM_RISCV
	/* It is a RISC-V ELF */
	return ehdr && (ehdr->e_machine == EM_RISCV);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_mipsle(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EM_MIPS
	/* It is a MIPS LSB ELF */
	return ehdr && (ehdr->e_machine == EM_MIPS) &&
	       (ehdr->e_ident[EI_DATA] == ELFDATA2LSB);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_powerpc(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EM_PPC
	/* It is a PowerPC ELF */
	return ehdr && (ehdr->e_machine == EM_PPC) &&
	       (ehdr->e_ident[EI_DATA] == ELFDATA2MSB);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_powerpc64(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EM_PPC64
	/* It is a PowerPC64 ELF */
	return ehdr && (ehdr->e_machine == EM_PPC64) &&
	       (ehdr->e_ident[EI_DATA] == ELFDATA2MSB);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_powerpc64le(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EM_PPC64
	/* It is a PowerPC64 LSB ELF */
	return ehdr && (ehdr->e_machine == EM_PPC64) &&
	       (ehdr->e_ident[EI_DATA] == ELFDATA2LSB);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_s390(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

#if defined EM_S390
	/* It is a IBM S/390 ELF */
	return ehdr && (ehdr->e_machine == EM_S390);
#else
	(void)ehdr;
	return __set_errno(ENOTSUP, -1);
#endif
}

static int __is_linux(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is a GNU/Linux ELF */
	return ehdr && (ehdr->e_ident[EI_OSABI] == ELFOSABI_GNU);
}

static int __is_gnu_linux(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	/* LINUX_$ARCH_$ABI or it is a GNU/Linux ELF */
	return __is_linux_ldso(ldso) == 1 || __is_linux(ehdr, ldso, abi) == 1;
}

static int __is_gnu_linux_ext(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	int ret;

	/* LINUX_$ARCH_$ABI or it is a GNU/Linux ELF */
	ret = __is_gnu_linux(ehdr, ldso, abi) == 1;
	if (ret)
		return ret;

	/* It is an MIPS LSB ELF and IAMROOT_LIB_MIPSLE_1 */
	ret = __is_mipsle(ehdr, ldso, abi) == 1 && (*ldso == 0 && abi == 1) &&
	      __is_32_bits(ehdr, ldso, abi) == 1;
	if (ret)
		return ret;

	/* It is an MIPS64 LSB ELF and IAMROOT_LIB_MIPSLE_1 */
	ret = __is_mipsle(ehdr, ldso, abi) == 1 && (*ldso == 0 && abi == 1) &&
	      __is_64_bits(ehdr, ldso, abi) == 1;
	if (ret)
		return ret;

	/* It is a PowerPC ELF and IAMROOT_LIB_POWERPC_1 */
	ret = __is_powerpc(ehdr, ldso, abi) && (*ldso == 0 && abi == 1);
	if (ret)
		return ret;

	/* It is a PowerPC64 ELF and IAMROOT_LIB_POWERPC64_2 */
	ret = __is_powerpc64(ehdr, ldso, abi) == 1 && (*ldso == 0 && abi == 2);
	if (ret)
		return ret;

	/* It is a PowerPC64 LSB ELF and IAMROOT_LIB_POWERPC64LE_2 */
	ret = __is_powerpc64le(ehdr, ldso, abi) == 1 &&
	      (*ldso == 0 && abi == 2);
	if (ret)
		return ret;

	/* It is an IBM S/390 ELF and IAMROOT_LIB_S390X_1 */
	ret = __is_s390(ehdr, ldso, abi) == 1 && (*ldso == 0 && abi == 1);
	if (ret)
		return ret;

	/* It is something else */
	return 0;
}

static int __is_musl(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ehdr;
	(void)abi;

	/* MUSL_$ARCH_$ABI */
	return __is_musl_ldso(ldso) == 1;
}

static int __is_freebsd(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	/* It is a FreeBSD ELF or ELF_1 */
	return (ehdr && (ehdr->e_ident[EI_OSABI] == ELFOSABI_FREEBSD)) ||
	       (__is_elf_ldso(ldso) == 1 && abi == 1);
}

static int __is_netbsd(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	/* It is a NetBSD ELF or ELF */
	return (ehdr && (ehdr->e_ident[EI_OSABI] == ELFOSABI_NETBSD)) ||
	       (__is_elf_ldso(ldso) == 1 && abi == -1);
}

static int __is_openbsd(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is a OpenBSD ELF */
	return (ehdr && (ehdr->e_ident[EI_OSABI] == ELFOSABI_OPENBSD));
}

static const char *__machine(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	const int errno_save = errno;

	if (__is_x86(ehdr, ldso, abi) == 1) {
		if (__is_gnu_linux(ehdr, ldso, abi) == 1 ||
		    __is_musl(ehdr, ldso, abi) == 1)
			return __set_errno(errno_save, "i686");

		/* Assuming it is a *BSD */
		return __set_errno(errno_save, "i386");
	} else if (__is_x86_64(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "x86_64");
	} else if (__is_arm(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "arm");
	} else if (__is_armhf(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "armhf");
	} else if (__is_aarch64(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "aarch64");
	} else if (__is_aarch64_be(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "aarch64_be");
	} else if (__is_riscv(ehdr, ldso, abi) == 1) {
		if (__is_gnu_linux(ehdr, ldso, abi) == 1 ||
		    __is_musl(ehdr, ldso, abi) == 1)
			return __set_errno(errno_save, "riscv64");

		/* Assuming it is a *BSD */
		return __set_errno(errno_save, "riscv");
	} else if (__is_mipsle(ehdr, ldso, abi) == 1 &&
		   __is_32_bits(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "mipsle");
	} else if (__is_mipsle(ehdr, ldso, abi) == 1 &&
		   __is_64_bits(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "mips64le");
	} else if (__is_powerpc(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "powerpc");
	} else if (__is_powerpc64(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "powerpc64");
	} else if (__is_powerpc64le(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "powerpc64le");
	} else if (__is_s390(ehdr, ldso, abi) == 1) {
		return __set_errno(errno_save, "s390x");
	}

	/* Unsupported yet! */
	return __set_errno(ENOTSUP, NULL);
}

static const char *__multiarch(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	const int errno_save = errno;

	if (__is_gnu_linux_ext(ehdr, ldso, abi) != 1 || !__getmultiarch())
		return NULL;

	/* It is an x86 ELF */
	if (__is_x86(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/lib/i386-linux-gnu:/usr/lib/i386-linux-gnu:/lib:/usr/lib");

	/* It is an x86-64 ELF */
	if (__is_x86_64(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu:/lib:/usr/lib");

	/* It is an ARM ELF */
	if (__is_arm(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/lib/arm-linux-gnueabi:/usr/lib/arm-linux-gnueabi:/lib:/usr/lib");

	/* It is an ARM Hard Float ELF */
	if (__is_armhf(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/lib/arm-linux-gnueabihf:/usr/lib/arm-linux-gnueabihf:/lib:/usr/lib");

	/* It is an AArch64 LSB ELF */
	if (__is_aarch64(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/lib/aarch64-linux-gnu:/usr/lib/aarch64-linux-gnu:/lib:/usr/lib");

	/* It is an RISC-V lp64d ELF */
	if (__is_riscv(ehdr, ldso, abi) == 1 &&
	    __is_64_bits(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/lib/riscv64-linux-gnu:/usr/lib/riscv64-linux-gnu:/lib:/usr/lib");

	/* It is an MIPS LSB ELF */
	if (__is_mipsle(ehdr, ldso, abi) == 1 &&
	    __is_32_bits(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/usr/lib/mipsel-linux-gnu:/lib/mipsel-linux-gnu:/usr/lib:/lib");

	/* It is an MIPS64 LSB ELF */
	if (__is_mipsle(ehdr, ldso, abi) == 1 &&
	    __is_64_bits(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/usr/lib/mips64el-linux-gnuabi64:/lib/mips64el-linux-gnuabi64:/usr/lib:/lib");

	/* It is a PowerPC ELF */
	if (__is_powerpc(ehdr, ldso, abi))
		return "/usr/lib/powerpc-linux-gnu:/lib/powerpc-linux-gnu:/usr/lib:/lib";

	/* It is a PowerPC64 ELF */
	if (__is_powerpc64(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/usr/lib/powerpc64le-linux-gnu:/lib/powerpc64-linux-gnu:/usr/lib:/lib");

	/* It is a PowerPC64 LSB ELF */
	if (__is_powerpc64le(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/usr/lib/powerpc64le-linux-gnu:/lib/powerpc64le-linux-gnu:/usr/lib:/lib");

	/* It is an IBM S/390 ELF */
	if (__is_s390(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/usr/lib/s390x-linux-gnu:/lib/s390x-linux-gnu:/usr/lib:/lib");

	/* It is something else */
	return __set_errno(ENOTSUP, NULL);
}

ssize_t __getvariable(Elf64_Ehdr *ehdr, const char *ldso, int abi,
		      const char *name, char *buf, ssize_t bufsiz)
{
	const int errno_save = errno;
	const char *machine;
	int n;

	/* Get machine name */
	machine = __machine(ehdr, ldso, abi);
	if (!machine)
		__warning("%i: ELF's machine is not supported yet!\n",
			  ehdr->e_machine);

	/* <VARNAME>_<MACHINE>_<LDSO>_<ABI> */
	if (machine && ldso && *ldso && abi != -1)
		n = _snprintf(buf, bufsiz, "%s_%s_%s_%i", name, machine, ldso,
			      abi);
	/* <VARNAME>_<MACHINE>_<LDSO> */
	else if (machine && ldso && *ldso && abi == -1)
		n = _snprintf(buf, bufsiz, "%s_%s_%s", name, machine, ldso);
	/* <VARNAME>_<MACHINE>_<ABI> */
	else if (machine && abi != -1)
		n = _snprintf(buf, bufsiz, "%s_%s_%i", name, machine, abi);
	/* <VARNAME>_<MACHINE> */
	else if (machine)
		n = _snprintf(buf, bufsiz, "%s_%s", name, machine);
	/* <VARNAME>_<LDSO>_<ABI> */
	else if (ldso && *ldso && abi != -1)
		n = _snprintf(buf, bufsiz, "%s_%s_%i", name, ldso, abi);
	/* <VARNAME>_<LDSO> */
	else if (ldso && *ldso && abi == -1)
		n = _snprintf(buf, bufsiz, "%s_%s", name, ldso);
	/* <VARNAME>_<ABI> */
	else if (abi != -1)
		n = _snprintf(buf, bufsiz, "%s_%i", name, abi);
	/* <VARNAME> */
	else
		n = _snprintf(buf, bufsiz, "%s", name);
	if (n == -1)
		return -1;

	__env_sanitize(buf, 1);
	return __set_errno(errno_save, n);
}

static const char *__getdeflib(Elf64_Ehdr *, const char *, int);

static ssize_t __libiamroot_access(Elf64_Ehdr *ehdr, const char *ldso, int abi,
				   int mode, const char *path, char *buf,
				   size_t bufsiz, off_t offset)
{
	char file[PATH_MAX];
	const char *machine;
	ssize_t siz;
	int n;

	machine = __machine(ehdr, ldso, abi);
	/* The machine is not supported; use the host */
	if (!machine)
		goto host;

	/* Generic dynamic loader (ld.so) */
	if (!*ldso && abi == -1)
		n = __snprintf(file, "%s/libiamroot.so", machine);
	/* Former GNU/Linux dynamic loader (ld.so.<abi>) */
	else if (!*ldso && abi != -1)
		n = __snprintf(file, "%s/libiamroot.so.%i", machine, abi);
	/* NetBSD dynamic loader (ld.<object>_so) */
	else if (*ldso && abi == -1)
		n = __snprintf(file, "%s/libiamroot.elf_so", machine);
	/* Linux or FreeBSD dynamic loader (ld-<ldso>.so.<abi>) */
	else
		n = __snprintf(file, "%s/libiamroot-%s.so.%i", machine, ldso,
			       abi);
	if (n == -1)
		return -1;

	siz = __host_path_access(file, mode, path, buf, bufsiz, offset);
	if (siz != -1)
		return siz;

host:
	__strncpy(file, "libiamroot.so");
	return __host_path_access(file, mode, path, buf, bufsiz, offset);
}

static ssize_t __getlibiamroot(Elf64_Ehdr *ehdr, const char *ldso, int abi,
			       char *buf, size_t bufsiz, off_t offset)
{
	const int errno_save = errno;
	const char *deflib, *origin;
	char var[PATH_MAX];
	ssize_t ret;
	char *lib;
	int err;

	/*
	 * Use the library set by the environment variable if set.
	 */
	ret = __getvariable(ehdr, ldso, abi, "IAMROOT_LIB", var, sizeof(var));
	if (ret == -1)
		return -1;

	lib = _getenv(var);
	if (lib)
		goto exit;

	/* The variable is unset, try to guess automagically the library. */

	/*
	 * Try in the iamroot directory set by the environment variable
	 * IAMROOT_ORIGIN if set, and in the iamroot library directory then.
	 */
	origin = _getenv("IAMROOT_ORIGIN");
	if (origin) {
		ssize_t siz;

		siz = __libiamroot_access(ehdr, ldso, abi, F_OK, origin, buf,
					  bufsiz, offset);
		if (siz > 0)
			lib = &buf[offset];
		if (lib && *lib)
			goto exit;
	}
	deflib = __xstr(PREFIX)"/lib/iamroot";
	if (deflib) {
		ssize_t siz;

		siz = __libiamroot_access(ehdr, ldso, abi, F_OK, deflib, buf,
					  bufsiz, offset);
		if (siz > 0)
			lib = &buf[offset];
		if (lib && *lib)
			goto exit;
	}

	/*
	 * The library is neither found in the iamroot origin library nor in
	 * the iamroot library directory.
	 *
	 * Use hard coded path by architecture and system.
	 */

	/* IAMROOT_LIB_$MACHINE_LINUX_$ARCH_$ABI */
	if (__is_gnu_linux(ehdr, ldso, abi) == 1) {
		/* It is an x86 ELF or IAMROOT_LIB_I686_LINUX_2 */
		if (__is_x86(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "linux") && abi == 2)) {
			lib = __xstr(PREFIX)"/lib/iamroot/i686/libiamroot-linux.so.2";
			goto access;
		}

		/* It is an x86-64 ELF or IAMROOT_LIB_X86_64_LINUX_X86_64_2 */
		if (__is_x86_64(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "linux-x86-64") && abi == 2)) {
			lib = __xstr(PREFIX)"/lib/iamroot/x86_64/libiamroot-linux-x86-64.so.2";
			goto access;
		}

		/* It is an ARM ELF or IAMROOT_LIB_ARM_LINUX_3 */
		if (__is_arm(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "linux") && abi == 3)) {
			lib = __xstr(PREFIX)"/lib/iamroot/arm/libiamroot-linux.so.3";
			goto access;
		}

		/* It is an ARMHF ELF or IAMROOT_LIB_ARMHF_LINUX_ARMHF_3 */
		if (__is_armhf(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "linux-armhf") && abi == 3)) {
			lib = __xstr(PREFIX)"/lib/iamroot/armhf/libiamroot-linux-armhf.so.3";
			goto access;
		}

		/* It is an AArch64 LSB ELF or IAMROOT_LIB_AARCH64_LINUX_AARCH64_1 */
		if (__is_aarch64(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "linux-aarch64") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/aarch64/libiamroot-linux-aarch64.so.1";
			goto access;
		}

		/* It is an AArch64 MSB ELF or IAMROOT_LIB_AARCH64_BE_LINUX_AARCH64_BE_1 */
		if (__is_aarch64_be(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "linux-aarch64_be") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/aarch64_be/libiamroot-linux-aarch64_be.so.1";
			goto access;
		}

		/* It is an RISC-V lp64d ELF or IAMROOT_LIB_RISCV64_LINUX_RISCV64_LP64D_1 */
		if ((__is_riscv(ehdr, ldso, abi) == 1 &&
		     __is_64_bits(ehdr, ldso, abi) == 1) ||
		    (streq(ldso, "linux-riscv64-lp64d") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/riscv64/libiamroot-linux-riscv64-lp64d.so.1";
			goto access;
		}
	}

	if (__is_mipsle(ehdr, ldso, abi) == 1 && (*ldso == 0 && abi == 1)) {
		/* It is an MIPS LSB ELF and IAMROOT_LIB_MIPSLE_1 */
		if (__is_32_bits(ehdr, ldso, abi) == 1) {
			lib = __xstr(PREFIX)"/lib/iamroot/mipsle/libiamroot.so.1";
			goto access;
		}

		/* It is an MIPS64 LSB ELF and IAMROOT_LIB_MIPS64LE_1 */
		if (__is_64_bits(ehdr, ldso, abi) == 1) {
			lib = __xstr(PREFIX)"/lib/iamroot/mips64le/libiamroot.so.1";
			goto access;
		}
	}

	/* It is a PowerPC ELF and IAMROOT_LIB_POWERPC_1 */
	if (__is_powerpc(ehdr, ldso, abi) && (*ldso == 0 && abi == 1)) {
		lib = __xstr(PREFIX)"/lib/iamroot/powerpc/libiamroot.so.1";
		goto access;
	}

	/* It is a PowerPC64 ELF and IAMROOT_LIB_POWERPC64_2 */
	if (__is_powerpc64(ehdr, ldso, abi) == 1 && (*ldso == 0 && abi == 2)) {
		lib = __xstr(PREFIX)"/lib/iamroot/powerpc64/libiamroot.so.2";
		goto access;
	}

	/* It is a PowerPC64 LSB ELF and IAMROOT_LIB_POWERPC64LE_2 */
	if (__is_powerpc64le(ehdr, ldso, abi) == 1 &&
	    (*ldso == 0 && abi == 2)) {
		lib = __xstr(PREFIX)"/lib/iamroot/powerpc64le/libiamroot.so.2";
		goto access;
	}

	/* It is an IBM S/390 ELF and IAMROOT_LIB_S390X_1 */
	if (__is_s390(ehdr, ldso, abi) == 1 && (*ldso == 0 && abi == 1)) {
		lib = __xstr(PREFIX)"/lib/iamroot/s390x/libiamroot.so.1";
		goto access;
	}

	/* IAMROOT_LIB_$MACHINE_MUSL_$ARCH_$ABI */
	if (__is_musl(ehdr, ldso, abi) == 1) {
		/* It is an x86 ELF or IAMROOT_LIB_I686_MUSL_I386_1 */
		if (__is_x86(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "musl-i386") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/i686/libiamroot-musl-i386.so.1";
			goto access;
		}

		/* It is an x86-64 ELF or IAMROOT_LIB_X86_64_MUSL_X86_64_1 */
		if (__is_x86_64(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "musl-x86_64") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/x86_64/libiamroot-musl-x86_64.so.1";
			goto access;
		}

		/* It is an ARM ELF or IAMROOT_LIB_ARM_MUSL_ARM_1 */
		if (__is_arm(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "musl-arm") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/arm/libiamroot-musl-arm.so.1";
			goto access;
		}

		/* It is an ARMHF ELF or IAMROOT_LIB_ARMHF_MUSL_ARMHF_1 */
		if (__is_armhf(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "musl-armhf") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/armhf/libiamroot-musl-armhf.so.1";
			goto access;
		}

		/* It is an AArch64 LSB ELF or IAMROOT_LIB_AARCH64_MUSL_AARCH64_1 */
		if (__is_aarch64(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "musl-aarch64") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/aarch64/libiamroot-musl-aarch64.so.1";
			goto access;
		}

		/* It is an AArch64 MSB ELF or IAMROOT_LIB_AARCH64_BE_MUSL_AARCH64_BE_1 */
		if (__is_aarch64(ehdr, ldso, abi) == 1 ||
		    (streq(ldso, "musl-aarch64_be") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/aarch64_be/libiamroot-musl-aarch64_be.so.1";
			goto access;
		}

		/* It is an RISC-V ELF or IAMROOT_LIB_RISCV64_MUSL_RISCV64_1 */
		if ((__is_riscv(ehdr, ldso, abi) == 1 &&
		     __is_64_bits(ehdr, ldso, abi) == 1) ||
		    (streq(ldso, "musl-riscv64") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/riscv64/libiamroot-musl-riscv64.so.1";
			goto access;
		}

		/* It is a MIPS LSB ELF or IAMROOT_LIB_MIPSLE_MUSL_MIPSEL_1 */
		if (__is_mipsle(ehdr, ldso, abi) == 1 &&
		    __is_32_bits(ehdr, ldso, abi) == 1 &&
		    (streq(ldso, "musl-mipsel") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/mipsle/libiamroot-musl-mipsel.so.1";
			goto access;
		}

		/* It is a MIPS64 LSB ELF or IAMROOT_LIB_MIPS64LE_MUSL_MIPS64EL_1 */
		if (__is_mipsle(ehdr, ldso, abi) == 1 &&
		    __is_64_bits(ehdr, ldso, abi) == 1 &&
		    (streq(ldso, "musl-mips64el") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/mips64le/libiamroot-musl-mips64el.so.1";
			goto access;
		}

		/* It is a PowerPC ELF and IAMROOT_LIB_POWERPC_MUSL_POWERPC_1 */
		if (__is_powerpc(ehdr, ldso, abi) == 1 &&
		    (streq(ldso, "musl-powerpc") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/powerpc/libiamroot-musl-powerpc.so.1";
			goto access;
		}

		/* It is a PowerPC64 ELF and IAMROOT_LIB_POWERPC64_MUSL_POWERPC64_1 */
		if (__is_powerpc64(ehdr, ldso, abi) == 1 &&
		    (streq(ldso, "musl-powerpc64") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/powerpc64/libiamroot-musl-powerpc64.so.1";
			goto access;
		}

		/* It is a PowerPC64 LSB ELF and IAMROOT_LIB_POWERPC64LE_MUSL_POWERPC64LE_1 */
		if (__is_powerpc64le(ehdr, ldso, abi) == 1 &&
		    (streq(ldso, "musl-powerpc64le") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/powerpc64le/libiamroot-musl-powerpc64le.so.1";
			goto access;
		}

		/* It is an IBM S/390 ELF or IAMROOT_LIB_S390X_MUSL_S390X_1 */
		if (__is_s390(ehdr, ldso, abi) == 1 &&
		    (streq(ldso, "musl-s390x") && abi == 1)) {
			lib = __xstr(PREFIX)"/lib/iamroot/s390x/libiamroot-musl-s390x.so.1";
			goto access;
		}
	}

	/* It is a FreeBSD ELF or IAMROOT_LIB_$MACHINE_ELF_1 */
	if (__is_freebsd(ehdr, ldso, abi) == 1) {
		/* It is an x86-64 ELF */
		if (__is_x86_64(ehdr, ldso, abi) == 1) {
			lib = __xstr(PREFIX)"/lib/iamroot/amd64/libiamroot-elf.so.1";
			goto access;
		}

		/* It is an AArch64 ELF */
		if (__is_aarch64(ehdr, ldso, abi) == 1) {
			lib = __xstr(PREFIX)"/lib/iamroot/arm64/libiamroot-elf.so.1";
			goto access;
		}
	}

	/* It is a NetBSD ELF or IAMROOT_LIB_$MACHINE_ELF */
	if (__is_netbsd(ehdr, ldso, abi) == 1) {
		/* It is an x86-64 ELF */
		if (__is_x86_64(ehdr, ldso, abi) == 1) {
			lib = __xstr(PREFIX)"/lib/iamroot/amd64/libiamroot.elf_so";
			goto access;
		}

		/* It is an AArch64 ELF */
		if (__is_aarch64(ehdr, ldso, abi) == 1) {
			lib = __xstr(PREFIX)"/lib/iamroot/arm64/libiamroot.elf_so";
			goto access;
		}
	}

	/* It is something else */
	lib = __xstr(PREFIX)"/lib/iamroot/libiamroot.so";

access:
	err = next_faccessat(AT_FDCWD, lib, X_OK, AT_EACCESS);
	if (err != -1)
		goto exit;

	/* The library is not found. */

	__notice("%s: No such ELF%i iamroot library for machine %i! Trying IAMROOT_LIB if set...\n",
		 lib, __is_64_bits(ehdr, ldso, abi) == 1 ? 64 : 32,
		 ehdr->e_machine);

	/*
	 * Use the library set by the environment variable IAMROOT_LIB if set.
	 */
	lib = _getenv("IAMROOT_LIB");
	if (lib)
		goto exit;

	/* The variable is unset; use the default value. */
	lib = __xstr(PREFIX)"/lib/iamroot/libiamroot.so";

	__warning("%s: No such ELF%i iamroot library for machine %i! Trying the default...\n",
		  lib, __is_64_bits(ehdr, ldso, abi) == 1 ? 64 : 32,
		  ehdr->e_machine);

exit:
	_strncpy(buf+offset, lib, bufsiz-offset);
	ret = strnlen(buf, bufsiz);

	return __set_errno(errno_save, ret);
}

static int __felf_interp_ldso_abi(int fd, Elf64_Ehdr *ehdr, char *interp,
				  size_t interpsiz, char *ldso, int *abi)
{
	ssize_t siz;

	/*
	 * According to execve(2):
	 *
	 * If the executable is a dynamically linked ELF executable, the
	 * interpreter named in the PT_INTERP segment is used to load the
	 * needed shared objects. This interpreter is typically
	 * /lib/ld-linux.so.2 for binaries linked with glibc (see
	 * ld-linux.so(8)).
	 */

	/* Get the ELF header... */
	siz = __felf_header(fd, ehdr);
	if (siz == -1)
		return -1;

	/*
	 * ... get the dynamic loader stored in the .interp section of the ELF
	 * linked program...
	 */
	siz = __felf_interp(fd, interp, interpsiz);
	if (siz < 1)
		return -1;

	/* ... and get its LDSO-name and its ABI number */
	return __ld_ldso_abi(interp, ldso, abi);
}

static int __elf_interp_ldso_abi(const char *path, Elf64_Ehdr *ehdr,
				 char *interp, size_t interpsiz, char *ldso,
				 int *abi)
{
	int fd, ret;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __felf_interp_ldso_abi(fd, ehdr, interp, interpsiz, ldso, abi);

	__close(fd);

	return ret;
}

hidden int __preload_libiamroot()
{
	char buf[PATH_MAX], hashbang[NAME_MAX], interp[NAME_MAX],
	     ldso[NAME_MAX];
	int fd, abi = -1, ret = -1;
	const char *path;
	Elf64_Ehdr ehdr;
	ssize_t siz;

	/* Get the executable */
	path = __execfn();
	if (!path)
		return -1;

	/* Get the interpreter if executable is an interpreter-script */
	siz = __interpreter_script_hashbang(path, hashbang, sizeof(hashbang),
					    0);
	if ((siz == -1) && (errno != ENOEXEC))
		return -1;
	if (siz > 0)
		path = hashbang;

	/* Open the executable file... */
	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	/*
	 * Get the dynamic loader stored in the .interp section of the ELF
	 * program, its LDSO-name and its ABI number
	 */
	ret = __felf_interp_ldso_abi(fd, &ehdr, interp, sizeof(interp), ldso,
				     &abi);
	if (ret == -1)
		goto close;

	/* Get the iamroot library */
	siz = __getlibiamroot(&ehdr, ldso, abi, buf, sizeof(buf), 0);
	if (siz == -1)
		goto close;

	/* Preload library if not preloaded */
	ret = __preload_so(buf);

close:
	__close(fd);

	return ret;
}

static const char *__getdeflib(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	const int errno_save = errno;
	char buf[NAME_MAX];
	const char *ret;
	ssize_t siz;

	/* Use the multiarch library path if IAMROOT_MULTIARCH is set. */
	ret = __multiarch(ehdr, ldso, abi);
	if (ret)
		return ret;

	/*
	 * Use the defaul library path set by the environment variable if set.
	 */
	siz = __getvariable(ehdr, ldso, abi, "IAMROOT_DEFLIB", buf,
			    sizeof(buf));
	if (siz == -1)
		return NULL;

	ret = _getenv(buf);
	if (ret)
		return __set_errno(errno_save, ret);

	/*
	 * Use the default library path set by the environment variable
	 * IAMROOT_DEFLIB if set.
	 */
	ret = _getenv("IAMROOT_DEFLIB");
	if (ret)
		return __set_errno(errno_save, ret);

	/*
	 * The variable is unset, try to guess automagically the standard
	 * library path for the GNU/Linux systems.
	 *
	 * According to ld-linux.so(8):
	 *
	 * In the default path /lib, and then /usr/lib.
	 *
	 * On some 64-bit architectures, the default paths for 64-bit shared
	 * objects are /lib64, and then /usr/lib64.
	 */
	if (__is_gnu_linux(ehdr, ldso, abi) == 1) {
		/* It is a 64-bit GNU/Linux */
		if (__is_64_bits(ehdr, ldso, abi) == 1)
			return __set_errno(errno_save, "/lib64:/usr/lib64");

		/* It is a GNU/Linux */
		return __set_errno(errno_save, "/lib:/usr/lib");
	}

	/* It is a NetBSD ELF or ELF */
	if (__is_netbsd(ehdr, ldso, abi) == 1)
		return __set_errno(errno_save, "/lib:/usr/pkg/lib:/usr/lib");

	/* It is something else */
	return __set_errno(errno_save, "/lib:/usr/local/lib:/usr/lib");
}

static const char *__getinhibit_rpath()
{
	return _getenv("IAMROOT_INHIBIT_RPATH") ?: "";
}

static ssize_t __ld_lib_path(const char *, char *, size_t, off_t);

/*
 * Note: This resolves all the needed shared objects to preload in order to
 * prevent from loading the shared objects from the host system.
*/
static ssize_t __ld_preload(Elf64_Ehdr *ehdr, const char *ldso, int abi,
			    const char *path, char *buf, size_t bufsiz,
			    off_t offset)
{
	char lib_path[PATH_MAX];
	ssize_t siz;
	int err;

	siz = __getlibiamroot(ehdr, ldso, abi, buf, bufsiz, offset);
	if (siz == -1)
		return -1;

	siz = __ld_lib_path(path, lib_path, sizeof(lib_path), 0);
	if (siz == -1)
		return -1;

	err = __ldso_preload_executable(path, lib_path, buf, bufsiz, offset);
	if (err == -1)
		return -1;

	return strnlen(buf+offset, bufsiz-offset);
}

static int __secure_execution_mode()
{
#if defined(__NetBSD__)
	return 0;
#else
	uid_t ruid, euid, suid;
	gid_t rgid, egid, sgid;
	int ret;

	/*
	 * Secure-execution mode
	 *
	 * For security reasons, if the dynamic linker determines that a binary
	 * should be run in secure-execution mode, the effects of some
	 * environment variables are voided or modified, and furthermore those
	 * environment variables are stripped from the environment, so that the
	 * program does not even see the definitions. Some of these environment
	 * variables affect the operation of the dynamic linker itself, and are
	 * described below. Other environment variables treated in this way
	 * include: GCONV_PATH, GETCONF_DIR, HOSTALIASES, LOCALDOMAIN, LOCPATH,
	 * MALLOC_TRACE, NIS_PATH, NLSPATH, RESOLV_HOST_CONF, RES_OPTIONS,
	 * TMPDIR, and TZDIR.
	 *
	 * A binary is executed in secure-execution mode if the AT_SECURE entry
	 * in the auxiliary vector (see getauxval(3)) has a nonzero value. This
	 * entry may have a non‐zero value for various reasons, including:
	 *
	 * •  The process's real and effective user IDs differ, or the real and
	 * effective group IDs differ. This typically occurs as a result of
	 * executing a set-user-ID or set-group-ID program.
	 *
	 * •  A process with a non-root user ID executed a binary that
	 * conferred capabilities to the process.
	 *
	 * •  A nonzero value may have been set by a Linux Security Module.
	 */
	ret = getresuid(&ruid, &euid, &suid);
	if (ret == -1)
		return -1;

	ret = getresgid(&rgid, &egid, &sgid);
	if (ret == -1)
		return -1;

	return ruid != euid || rgid != egid;
#endif
}

/*
 * Note: This resolves all the library path of the executable file in order to
 * prevent from loading the shared objects of the host system.
*/
static ssize_t __fld_lib_path(int fd, char *buf, size_t bufsiz, off_t offset)
{
	int err, has_runpath;
	char tmp[PATH_MAX];
	uint32_t flags_1;
	char *str;

	buf[offset] = 0;

	/*
	 * According to ld-linux.so(8)
	 *
	 * (5)  In the default path /lib, and then /usr/lib. (On some 64-bit
	 * architectures, the default paths for 64-bit shared objects are
	 * /lib64, and then /usr/lib64.) If the binary was linked with the
	 * -z nodeflib linker option, this step is skipped.
	 */
	err = __felf_flags_1(fd, &flags_1);
	if (err == -1)
		return -1;
	if (!(flags_1 & DF_1_NODEFLIB)) {
		err = __felf_deflib(fd, tmp, sizeof(tmp), 0);
		if (err == -1)
			return -1;

		str = __path_strncat(buf+offset, tmp, bufsiz-offset);
		if (!str)
			return -1;
	}

	/*
	 * (4)  From the cache file /etc/ld.so.cache, which contains a compiled
	 * list of candidate shared objects previously found in the augmented
	 * library path. If, however, the binary was linked with the -z
	 * nodeflib linker option, shared objects in the default paths are
	 * skipped. Shared objects installed in hardware capability directories
	 * are preferred to other shared objects.
	 */
	/* TODO: This is not applicable, at least for now. */

	/*
	 * (3)  Using the directories specified in the DT_RUNPATH dynamic
	 * section attribute of the binary if present. Such directories are
	 * searched only to find those objects required by DT_NEEDED (direct
	 * dependencies) entries and do not apply to those objects' children,
	 * which must themselves have their own DT_RUNPATH entries. This is
	 * unlike DT_RPATH, which is applied to searches for all children in
	 * the dependency tree.
	 */
	err = __felf_runpath(fd, tmp, sizeof(tmp), 0);
	if (err == -1)
		return -1;

	has_runpath = err > 0;
	if (has_runpath) {
		str = __path_strncat(buf+offset, tmp, bufsiz-offset);
		if (!str)
			return -1;
	}

	/*
	 * (2)  Using the environment variable LD_LIBRARY_PATH, unless the
	 * executable is being run in secure-execution mode (see below), in
	 * which case this variable is ignored.
	 */
	if (!__secure_execution_mode()) {
		err = __getld_library_path(tmp, sizeof(tmp), 0);
		if (err == -1)
			return -1;

		str = __path_strncat(buf+offset, tmp, bufsiz-offset);
		if (!str)
			return -1;
	}

	/*
	 * (1)  Using the directories specified in the DT_RPATH dynamic section
	 * attribute of the binary if present and DT_RUNPATH attribute does not
	 * exist. Use of DT_RPATH is deprecated.
	 */
	if (!has_runpath) {
		err = __felf_rpath(fd, tmp, sizeof(tmp), 0);
		if (err == -1)
			return -1;

		str = __path_strncat(buf+offset, tmp, bufsiz-offset);
		if (!str)
			return -1;
	}

	return strnlen(buf+offset, bufsiz-offset);
}

static ssize_t __ld_lib_path(const char *path, char *buf, size_t bufsiz,
			     off_t offset)
{
	int fd, ret;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fld_lib_path(fd, buf, bufsiz, offset);

	__close(fd);

	return ret;
}

struct __ldso_library_path_context {
	const char *root;
	char *buf;
	size_t bufsiz;
	off_t offset;
};

static int __ldso_library_path_callback(const char *path, void *user)
{
	struct __ldso_library_path_context *ctx =
				    (struct __ldso_library_path_context *)user;
	char buf[PATH_MAX];
	char *str;
	int n;

	/* Prepend the root directory to path first */
	n = _snprintf(buf, sizeof(buf), "%s%s", ctx->root, path);
	if (n == -1)
		return -1;

	/* Add the chroot'ed path to buf then */
	str = __path_strncat(ctx->buf+ctx->offset, buf, ctx->bufsiz-ctx->offset);
	if (!str)
		return -1;

	return 0;
}

static ssize_t __ld_library_path(const char *path, char *buf, size_t bufsiz,
				 off_t offset)
{
	struct __ldso_library_path_context ctx;
	char lib_path[PATH_MAX];
	ssize_t siz;
	int err;

	/* Prepend the path from the library path by the root directory */
	siz = __ld_lib_path(path, lib_path, sizeof(lib_path), 0);
	if (siz == -1)
		return -1;
	ctx.root = __getrootdir();
	ctx.buf = buf;
	ctx.bufsiz = bufsiz;
	ctx.offset = offset;
	err = __path_iterate(lib_path, __ldso_library_path_callback, &ctx);
	if (err == -1)
		return -1;

	return strnlen(buf+offset, bufsiz-offset);
}

static int __flags_callback(const void *data, size_t datasiz, void *user)
{
	uint32_t *flags = (uint32_t *)data;
	uint32_t *f = (uint32_t *)user;
	(void)datasiz;

	if (!data || !user)
		return __set_errno_and_perror(EINVAL, -1);

	*f = *flags;

	return 0;
}

static int __felf_flags_1(int fd, uint32_t *flags)
{
	*flags = 0;
	return __elf_iterate_shared_object(fd, DT_FLAGS_1, __flags_callback,
					   flags);
}

static int __path_callback(const void *data, size_t datasiz, void *user)
{
	const char *path = (const char *)data;
	char *str, *p = (char *)user;
	(void)datasiz;

	if (!data || !user)
		return __set_errno_and_perror(EINVAL, -1);

	str = __path_strncat(p, path, PATH_MAX);
	if (!str)
		return -1;

	return 0;
}

static ssize_t __felf_needed(int fd, char *buf, size_t bufsiz)
{
	int err;

	*buf = 0;
	err = __elf_iterate_shared_object(fd, DT_NEEDED, __path_callback, buf);
	if (err == -1)
		return -1;

	return strnlen(buf, bufsiz);
}

static ssize_t __elf_needed(const char *path, char *buf, size_t bufsiz)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __felf_needed(fd, buf, bufsiz);

	__close(fd);

	return ret;
}

static ssize_t __felf_rpath(int fd, char *buf, size_t bufsiz, off_t offset)
{
	int err, n;

	buf[offset] = 0;
	err = __elf_iterate_shared_object(fd, DT_RPATH, __path_callback,
					  buf+offset);
	if (err == -1)
		return -1;

	n = __variable_has_dynamic_string_tokens(buf+offset);
	if (n)
		__warning("%s: RPATH has dynamic %i string token(s): %s\n",
			  __fpath(fd), n, buf+offset);

	return strnlen(buf+offset, bufsiz-offset);
}

static ssize_t __felf_runpath(int fd, char *buf, size_t bufsiz, off_t offset)
{
	int err, n;

	buf[offset] = 0;
	err = __elf_iterate_shared_object(fd, DT_RUNPATH, __path_callback,
					  buf+offset);
	if (err == -1)
		return -1;

	n = __variable_has_dynamic_string_tokens(buf+offset);
	if (n)
		__warning("%s: RUNPATH has dynamic %i string token(s): %s\n",
			  __fpath(fd), n, buf+offset);

	return strnlen(buf+offset, bufsiz-offset);
}

static int __has_needed_callback(const char *needed, void *user)
{
	const char *filename = (const char *)user;

	return __strleq(needed, filename);
}

static int __has_needed(int fd, const char *filename)
{
	char needed[PATH_MAX];
	ssize_t siz;

	siz = __felf_needed(fd, needed, sizeof(needed));
	if (siz == -1)
		return -1;

	/* Search for the needed shared object */
	return __path_iterate(needed, __has_needed_callback, (void *)filename);
}

static int __getld_linux_so_callback(const char *needed, void *user)
{
	char *interp = (char *)user;

	/* It is not the linux dynamic loader */
	if (__strneq(needed, "ld-linux") == 0)
		return 0;

	_strncpy(interp, needed, NAME_MAX);

	return 1;
}

static ssize_t __getld_linux_so(int fd, char *buf, size_t bufsiz)
{
	const char *libc_so_6 = "libc.so.6";
	char needed[PATH_MAX];
	char tmp[PATH_MAX];
	ssize_t siz;
	int err;

	/* Search for the linux C library (libc.so.6) first */
	err = __has_needed(fd, libc_so_6);
	if (err == -1)
		return -1;
	if (err == 0)
		return __set_errno(ENOENT, -1);

	siz = __felf_needed(fd, needed, sizeof(needed));
	if (siz == -1)
		return -1;

	siz = __path_access(libc_so_6, F_OK, "/lib:/lib64", tmp, sizeof(tmp));
	if (siz == -1)
		return -1;

	/* Search for the linux dynamic loader (ld-linux-$ARCH.so.$ABI) then */
	err = __path_iterate(needed, __getld_linux_so_callback, buf);
	if (err == -1)
		return -1;
	if (err == 0)
		return __set_errno(ENOENT, -1);

	return strnlen(buf, bufsiz);
}

static ssize_t __felf_deflib(int fd, char *buf, size_t bufsiz, off_t offset)
{
	const int errno_save = errno;
	char interp[NAME_MAX];
	char ldso[NAME_MAX];
	const char *deflib;
	int err, abi = 0;
	Elf64_Ehdr ehdr;
	ssize_t siz;

	/*
	 * Detects the default library path using the ELF header:
	 *  - the class (32-bit or 64-bit architectures)
	 *  - the operating system and ABI (UNIX System V, GNU or Linux alias,
	 *    FreeBSD ABIs)
	 * and using the interpreter in the PT_INTERP segment if the shared
	 * object is an executable file, or from the needed libc, or from the
	 * executable file.
	 */
	err = __felf_header(fd, &ehdr);
	if (err == -1)
		return -1;

	siz = __felf_interp(fd, interp, sizeof(interp));
	/* It is an ELF library */
	if (siz == -1 && errno == ENOEXEC) {
		__info("%s: dynamic loader not found, trying libc.so.6...\n",
		       __fpath(fd));
		siz = __getld_linux_so(fd, interp, sizeof(interp));
		if (siz == -1 && errno != ENOENT)
			return -1;
	}
	/* It is an ELF library linked against libc */
	if (siz == -1 && errno == ENOENT) {
		const char *path;

		path = __execfn();
		__info("%s: dynamic loader not found, trying executable file...\n",
		       path);
		siz = __elf_interp(path, interp, sizeof(interp));
		if (siz == -1 && errno != ENOENT)
			return -1;
	}
	if (siz < 1) {
		__warning("%s: dynamic loader not found!\n", __fpath(fd));
		*ldso = __set_errno(errno_save, 0);
		goto deflib;
	}

	err = __ld_ldso_abi(interp, ldso, &abi);
	if (err == -1)
		return -1;

deflib:
	deflib = __getdeflib(&ehdr, ldso, abi);
	if (!deflib)
		return -1;

	_strncpy(buf+offset, deflib, bufsiz-offset);

	return strnlen(buf+offset, bufsiz-offset);
}

static ssize_t __inhibit_rpath(char *buf, size_t bufsiz, off_t offset)
{
	_strncpy(buf+offset, __getinhibit_rpath(), bufsiz-offset);
	return strnlen(buf+offset, bufsiz-offset);
}

hidden int __ldso(const char *path, char * const argv[], char *interparg[],
		  char *buf, size_t bufsiz, off_t offset)
{
	int i, j, has_argv0 = 0, has_preload = 0, has_library_path = 0,
	    has_inhibit_rpath = 0, has_inhibit_cache = 0, shift = 0;
	char *argv0 = NULL, *inhibit_rpath = NULL, *ld_library_path = NULL,
	     *ld_preload = NULL;
	char pt_interp[NAME_MAX], ldso[NAME_MAX];
	int abi = 0, err = -1;
	char *interp = NULL;
	off_t off = offset;
	char * const *arg;
	Elf64_Ehdr ehdr;
	ssize_t siz;
	(void)argv;

	/*
	 * Get the dynamic loader stored in the .interp section of the ELF
	 * program, its LDSO-name and its ABI number
	 */
	err = __elf_interp_ldso_abi(path, &ehdr, pt_interp, sizeof(pt_interp),
				    ldso, &abi);
	if (err == -1)
		return -1;

	/*
	 * The dynamic load has to preload its libiamroot.so library, and
	 * eventually set an in-chroot library path and disable cache.
	 */

	/*
	 * The glibc world supports the options:
	 *  - --argv0 since 2.33
	 *  - --preload since 2.30
	 *  - --inhibit-cache since 2.16
	 *  - --inhibit-rpath since 2.0.94
	 *  - --library-path since 2.0.92
	 */
	if (__is_gnu_linux_ext(&ehdr, ldso, abi) == 1) {
		shift = 1;
		has_inhibit_rpath = 1;
		has_inhibit_cache =
				__ld_linux_has_inhibit_cache_option(pt_interp);
		if (has_inhibit_cache == -1)
			return -1;

		has_argv0 = __ld_linux_has_argv0_option(pt_interp);
		if (has_argv0 == -1)
			return -1;

		has_preload = __ld_linux_has_preload_option(pt_interp);
		if (has_preload == -1)
			return -1;

		has_library_path = 1;
	}

	/*
	 * The musl world supports the options:
	 *  - --argv0
	 *  - --preload
	 *  - --library-path
	 *
	 * It does not support neither --inhibit-cache (it has no cache support
	 * at all) nor --inhibit-rpath.
	 */
	if (__is_musl(&ehdr, ldso, abi) == 1) {
		shift = 1;
		has_argv0 = 1;
		has_preload = 1;
		has_library_path = 1;
	}

	/*
	 * The FreeBSD world supports none of them; it supports the environment
	 * variables LD_PRELOAD and LD_LIBRARY_PATH.
	 *
	 * According to ld-elf.so.1(1):
	 *
	 * LD_LIBRARY_PATH
	 * A colon separated list of directories, overriding the default search
	 * path for shared libraries. This variable is unset for set-user-ID
	 * and set-group-ID programs.
	 *
	 * LD_PRELOAD
	 * A list of shared libraries, separated by colons and/or white space,
	 * to be linked in before any other shared libraries. If the directory
	 * is not specified then the directories specified by LD_LIBRARY_PATH
	 * will be searched first followed by the set of built-in standard
	 * directories. This variable is unset for set-user-ID and
	 * set-group-ID programs.
	 *
	 * According to /libexec/ld-elf.so.1 -h:
	 *
	 * Usage: /libexec/ld-elf.so.1 [-h] [-b <exe>] [-d] [-f <FD>] [-p] [--] <binary> [<args>]
	 *
	 * Options:
	 *   -h        Display this help message
	 *   -b <exe>  Execute <exe> instead of <binary>, arg0 is <binary>
	 *   -d        Ignore lack of exec permissions for the binary
	 *   -f <FD>   Execute <FD> instead of searching for <binary>
	 *   -p        Search in PATH for named binary
	 *   -u        Ignore LD_ environment variables
	 *   -v        Display identification information
	 *   --        End of RTLD options
	 *   <binary>  Name of processto execute
	 *   <args>    Arguments to the executed process
	 */
	if (__is_freebsd(&ehdr, ldso, abi) == 1)
		shift = 1;

	/*
	 * The NetBSD and OpenBSD worlds support the environment variables
	 * LD_PRELOAD and LD_LIBRARY_PATH.
	 *
	 * The NetBSD world segfaults if run directly.
	 *
	 * According to ld.elf_so(1):
	 *
	 * If the following environment variables exist they will be used by
	 * ld.elf_so.
	 *
	 * LD_LIBRARY_PATH
	 * A colon separated list of directories, overriding the default search
	 * path for shared libraries.
	 *
	 * LD_PRELOAD
	 * A colon or space separated list of shared object filenames to be
	 * loaded after the main program but before its shared object
	 * dependencies. Space is allowed as a separator for backwards
	 * compatibility only. Support may be removed in a future release and
	 * should not be relied upon.
	 *
	 * The OpenBSD world has an none-executable ld.so.
	 *
	 * According to ld.so(1):
	 *
	 * ld.so recognises a number of environment variables that can be used
	 * to modify its behaviour as follows:
	 *
	 * LD_LIBRARY_PATH
	 * A colon separated list of directories, prepending the default search
	 * path for shared libraries. This variable is ignored for set-user-ID
	 * and set-group-ID executables.
	 *
	 * LD_PRELOAD
	 * A colon separated list of library names to load before any of the
	 * regular libraries are loaded. This variable is ignored for set-
	 * user-ID and set-group-ID executables.
	 */
	if (__is_netbsd(&ehdr, ldso, abi) == 1 ||
	    __is_openbsd(&ehdr, ldso, abi) == 1)
		shift = 0;

	siz = __ld_preload(&ehdr, ldso, abi, path, buf, bufsiz, off);
	if (siz == -1)
		return -1;
	if (siz > 0) {
		ld_preload = buf+off;
		off += siz+1; /* NULL-terminated */
	}

	siz = __ld_library_path(path, buf, bufsiz, off);
	if (siz == -1)
		return -1;
	if (siz > 0) {
		ld_library_path = buf+off;
		off += siz+1; /* NULL-terminated */
	}

	if (has_inhibit_rpath) {
		siz = __inhibit_rpath(buf, bufsiz, off);
		if (siz == -1)
			return -1;
		if (siz > 0) {
			inhibit_rpath = buf+off;
			off += siz+1; /* NULL-terminated */
		}
	}

	/*
	 * Clear the dynamic loader environment variables.
	 */

	err = _unsetenv("LD_PRELOAD");
	if (err == -1)
		return -1;

	err = _unsetenv("LD_LIBRARY_PATH");
	if (err == -1)
		return -1;

	siz = path_resolution(AT_FDCWD, pt_interp, &buf[off], bufsiz-off, 0);
	if (siz == -1)
		return -1;
	interp = &buf[off];
	off += siz+1; /* NULL-terminated */

	/*
	 * Shift enough room in interparg to prepend:
	 *   - the path to the interpreter (i.e. the absolute path in host,
	 *     including the chroot; argv0).
	 *   - the option --ld-preload and its argument (i.e. the path in host
	 *     environment to the iamroot library and the path in chroot
	 *     environment to the interpreter's libc.so and libdl.so to
	 *     preload).
	 *   - the option --library-path and its argument (i.e. the path in
	 *     chroot environment to the libraries paths).
	 *   - the option --inhibit-rpath and its argument (i.e. the path in
	 *     host environment to the libbraries to inhibit).
	 *   - the option --inhibit-cache.
	 *   - the option --argv0 and its argument (i.e. the original path in
	 *     host to the binary).
	 *   - the path to the binary (i.e. the full path in chroot, *not*
	 *     including chroot; first positional argument).
	 * Note: the binary's arguments are the original argv shifted by one
	 *       (i.e. without argv0; following arguments).
	 */
	argv0 = interparg[0];
	if (has_argv0)
		shift += 2;
	if (has_preload && ld_preload)
		shift += 2;
	if (has_library_path && ld_library_path)
		shift += 2;
	if (has_inhibit_rpath && inhibit_rpath)
		shift += 2;
	if (has_inhibit_cache)
		shift++;
	i = 0;
	for (arg = interparg; *arg; arg++)
		i++;
	for (j = i+shift; j > shift; j--)
		interparg[j] = interparg[j-shift];
	j = i;

	/* Add path to interpreter (host, argv0) */
	i = 0;
	if (shift)
		interparg[i++] = interp;

	/*
	 * Add --preload and the libraries to preload:
	 *  - libiamroot.so (from host)
	 *  - libc.so, libdl.so and libpthread.so (from chroot)
	 *  - DT_NEEDED libraries of binary (from chroot)
	 */
	if (shift && has_preload && ld_preload) {
		interparg[i++] = "--preload";
		interparg[i++] = ld_preload;
	/* Or set LD_PRELOAD if --preload is not supported */
	} else if (ld_preload) {
		err = _setenv("LD_PRELOAD", ld_preload, 1);
		if (err == -1)
			return -1;
	}

	/*
	 * Add --library-path (chroot) and the library path:
	 *  - DT_RPATH of binary (from chroot)
	 *  - LD_LIBRARY_PATH of environment (from chroot)
	 *  - DT_RUNPATH of binary (from chroot)
	 *  - default library path of interpreter (from chroot)
	 */
	if (shift && has_library_path && ld_library_path) {
		interparg[i++] = "--library-path";
		interparg[i++] = ld_library_path;
	/* Or set LD_LIBRARY_PATH if --library-path is not supported */
	} else if (ld_library_path) {
		err = _setenv("LD_LIBRARY_PATH", ld_library_path, 1);
		if (err == -1)
			return -1;
	}

	/* Add --inhibit-rpath (chroot) */
	if (shift && has_inhibit_rpath && inhibit_rpath) {
		interparg[i++] = "--inhibit-rpath";
		interparg[i++] = inhibit_rpath;
	}

	/* Add --inhibit-cache */
	if (shift && has_inhibit_cache)
		interparg[i++] = "--inhibit-cache";

	/* Add --argv0 and original argv0 */
	if (shift && has_argv0) {
		interparg[i++] = "--argv0";
		interparg[i++] = argv0;
	} else {
		/*
		 * The dynamic loader does not support for the option
		 * --argv0; the value will be set by via the function
		 * __libc_start_main().
		 */
		err = _setenv("argv0", argv0, 1);
		if (err == -1)
			return -1;
	}

	/* Add path to binary (in chroot, first positional argument) */
	interparg[i] = (char *)path;
	i += j;
	interparg[i] = NULL; /* ensure NULL-terminated */

	return i;
}

/*
 * Get the DT_RPATH, DT_RUNPATH, LD_LIBRARY_PATH, DT_FLAGS_1, and the
 * default library path.
 */
static int __felf_so_context(int fd,
			     char **rpath,
			     char **ld_library_path,
			     char **runpath,
			     char **deflib,
			     uint32_t *flags_1,
			     char *buf,
			     size_t bufsiz,
			     off_t offset)
{
	off_t off = offset;
	ssize_t siz;

	if (fd < 0 || !rpath || !ld_library_path || !runpath || !deflib ||
	    !flags_1 || !buf)
		return __set_errno_and_perror(EINVAL, -1);

	*rpath = NULL;
	*ld_library_path = NULL;
	*runpath = NULL;
	*deflib = NULL;
	*flags_1 = 0;

	siz = __felf_runpath(fd, buf, bufsiz, off);
	if (siz == -1)
		return -1;
	if (siz > 0) {
		*runpath = buf+off;
		off += siz+1; /* NULL-terminated */
	}

	if (!*runpath) {
		siz = __felf_rpath(fd, buf, bufsiz, off);
		if (siz == -1)
			return -1;
		if (siz > 0) {
			*rpath = buf+off;
			off += siz+1; /* NULL-terminated */
		}
	}

	if (!__secure_execution_mode()) {
		siz = __getld_library_path(buf, bufsiz, off);
		if (siz == -1)
			return -1;
		if (siz > 0) {
			*ld_library_path = buf+off;
			off += siz+1; /* NULL-terminated */
		}
	}

	siz = __felf_deflib(fd, buf, bufsiz, off);
	if (siz == -1)
		return -1;
	if (siz > 0) {
		*deflib = buf+off;
		off += siz+1; /* NULL-terminated */
	}

	return __felf_flags_1(fd, flags_1);
}

static int __elf_so_context(const char *path,
			    char **rpath,
			    char **ld_library_path,
			    char **runpath,
			    char **deflib,
			    uint32_t *flags_1,
			    char *buf,
			    size_t bufsiz,
			    off_t offset)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __felf_so_context(fd, rpath, ld_library_path, runpath, deflib,
				flags_1, buf, bufsiz, offset);

	__close(fd);

	return ret;
}

hidden ssize_t __dl_access(const char *path, int mode, char *buf,
			   size_t bufsiz)
{
	char *deflib, *ld_library_path, *exec_rpath, *exec_runpath;
	char tmp[PATH_MAX];
	const char *execfn;
	uint32_t flags_1;
	int err;

	if (strchr(path, '/'))
		return __set_errno_and_perror(EINVAL, -1);

	/* Get the executable */
	execfn = __execfn();
	if (!execfn)
		return -1;

	/* Get the ELF shared object context */
	err = __elf_so_context(execfn, &exec_rpath, &ld_library_path,
			       &exec_runpath, &deflib, &flags_1, tmp,
			       sizeof(tmp), 0);
	if (err == -1)
		return -1;

	/* Look for the shared object */
	return __ldso_access(path, mode, exec_rpath, exec_runpath, NULL,
			     ld_library_path, NULL, deflib, flags_1, buf,
			     bufsiz);
}

#ifndef __NetBSD__
static const char *__root_basepath(const char *path)
{
	const char *root;
	size_t len;

	root = __getrootdir();
	if (streq(root, "/"))
		return NULL;

	len = __strlen(root);
	if (!strneq(root, path, len))
		return NULL;

	return path+len; /* root directory */
}

static int __getld_trace_loaded_objects()
{
	return strtol(_getenv("LD_TRACE_LOADED_OBJECTS") ?: "0", NULL, 0);
}

static int __ld_trace_loader_objects_needed(const char *, const char *,
					    const char *, const char *,
					    const char *, uint32_t, char *,
					    size_t);

struct __ld_trace_loader_objects_needed_context {
	char *buf;
	size_t bufsiz;
	FILE *f;
};

static int __ld_trace_loader_objects_needed_callback(const char *so,
						     const char *exec_rpath,
						     const char *ld_library_path,
						     const char *exec_runpath,
						     const char *deflib,
						     uint32_t flags_1,
						     void *user)
{
	struct __ld_trace_loader_objects_needed_context *ctx =
		       (struct __ld_trace_loader_objects_needed_context *)user;
	const ssize_t map_start = 0;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	/* Look for the shared object */
	siz = __ldso_access(so, F_OK, exec_rpath, exec_runpath, NULL,
			    ld_library_path, exec_runpath, deflib, flags_1,
			    buf, sizeof(buf));
	if (siz < 1)
		fprintf(ctx->f, "\t%s => not found\n", so);
	if (siz == -1)
		return 0;

	/* Ignore none-libraries (i.e. linux-vdso.so.1, ld.so-ish...) */
	ret = __is_lib(__basename(buf));
	if (ret == 0 && !__is_ldso(__basename(buf)))
		__warning("%s: ignoring non-library!\n", __basename(buf));
	if (ret == 0)
		return 0;

	/* The shared object is already in the buffer */
	ret = __is_in_path(buf, ctx->buf);
	if (ret == -1 || ret == 1)
		return 0;

	/* Add the needed shared objects to buf */
	ret = __ld_trace_loader_objects_needed(buf,
					       exec_rpath,
					       ld_library_path,
					       exec_runpath,
					       deflib,
					       flags_1,
					       ctx->buf,
					       ctx->bufsiz);

	fprintf(ctx->f, "\t%s => %s (0x%0*zx)\n", so, __root_basepath(buf),
		(int)sizeof(map_start) * 2, map_start);

	return ret;
}

static int __ld_trace_loader_objects_needed(const char *path,
					    const char *exec_rpath,
					    const char *ld_library_path,
					    const char *exec_runpath,
					    const char *deflib,
					    uint32_t flags_1,
					    char *buf,
					    size_t bufsiz)
{
	struct __ld_trace_loader_objects_needed_context ctx;
	char needed[PATH_MAX];
	ssize_t siz;
	char *str;
	int ret;

	siz = __elf_needed(path, needed, sizeof(needed));
	if (siz == -1)
		return -1;

	/* Add the needed shared objects to path first */
	ctx.buf = buf;
	ctx.bufsiz = bufsiz;
	ctx.f = stdout;
	ret = __ld_needed(path,
			  exec_rpath,
			  ld_library_path,
			  exec_runpath,
			  deflib,
			  flags_1,
			  __ld_trace_loader_objects_needed_callback,
			  &ctx);
	if (ret == -1)
		return -1;

	/* Add the shared object to path then */
	str = __path_strncat(buf, path, bufsiz);
	if (!str)
		return -1;

	return 0;
}

static int __fld_trace_loader_objects_executable(int fd)
{
	char *deflib, *exec_rpath, *exec_runpath, *ld_library_path;
	struct __ld_trace_loader_objects_needed_context ctx;
	const size_t map_start = 0;
	char interp[NAME_MAX];
	char ldso[NAME_MAX];
	char buf[PATH_MAX];
	char tmp[PATH_MAX];
	int ret, abi = 0;
	uint32_t flags_1;
	FILE *f = stdout;
	ssize_t siz;

	*buf = 0;

	/* Get the ELF shared object context */
	ret = __felf_so_context(fd, &exec_rpath, &ld_library_path,
				&exec_runpath, &deflib, &flags_1, tmp,
				sizeof(tmp), 0);
	if (ret == -1)
		return -1;

	/* Get the DT_INTERP */
	siz = __felf_interp(fd, interp, sizeof(interp));
	if (siz == -1 && errno != ENOEXEC)
		return -1;
	/* It has no DT_INTERP */
	if (siz == -1 && errno == ENOEXEC)
		fprintf(f, "\tstatically linked\n");
	if (siz == -1 && errno == ENOEXEC)
		return 0;

	/* Print the vDSO if GNU/Linux */
	ret = __ld_ldso_abi(interp, ldso, &abi);
	if (ret == -1)
		return -1;
	ret = __is_linux_ldso(ldso);
	if (ret == -1)
		return -1;
	if (ret == 1)
		fprintf(f, "\t%s (0x%0*zx)\n", "linux-vdso.1",
			(int)sizeof(map_start) * 2, map_start);

	/* Print the DT_NEEDED shared objects */
	ctx.buf = buf;
	ctx.bufsiz = sizeof(buf);
	ctx.f = f;
	ret = __fld_needed(fd,
			   exec_rpath,
			   ld_library_path,
			   exec_runpath,
			   deflib,
			   flags_1,
			   __ld_trace_loader_objects_needed_callback,
			   &ctx);

	/* Print the PT_INTERP interpreter */
	fprintf(f, "\t%s => %s (0x%0*zx)\n", interp, interp,
		(int)sizeof(map_start) * 2, map_start);

	return ret;
}

static int __ld_trace_loader_objects_executable(const char *path)
{
	int fd, ret;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fld_trace_loader_objects_executable(fd);

	__close(fd);

	return ret;
}

static int __ld_trace_loader_objects_exec(const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return -1;

	return __ld_trace_loader_objects_executable(buf);
}

static int __ldso_verify(const char *path)
{
	char interp[NAME_MAX];
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return -1;

	siz = __elf_interp(buf, interp, sizeof(interp));
	if (siz == -1)
		return -1;

	return 0;
}

hidden int __ldso_execve(const char *path, char * const argv[],
			 char * const envp[])
{
	int ret;
	(void)path;
	(void)envp;

	if (!argv[1])
		goto exit;

	/*
	 * According to ld-linux.so(8)
	 *
	 * --verify
	 *
	 * Verify that program is dynamically linked and this dynamic linker
	 * can handle it.
	 */
	if (streq(argv[1], "--verify")) {
		ret = __ldso_verify(argv[2]);
		if (ret == -1 && errno == ENOEXEC)
			_exit(2);
		if (ret == -1)
			_exit(1);

		_exit(0);
	}

	/*
	 * According to ld-linux.so(8)
	 *
	 * LD_TRACE_LOADED_OBJECTS
	 *
	 * If set (to any value), causes the program to list its dynamic
	 * dependencies, as if run by ldd(1), instead of running normally.
	 */
	ret = __getld_trace_loaded_objects();
	if (ret == -1)
		return -1;
	if (ret == 0)
		goto exit;

	ret = __ld_trace_loader_objects_exec(argv[1]);
	if (ret == -1)
		return -1;

	_exit(0);

exit:
	return __set_errno(EAGAIN, -1);
}

hidden int __ldso_execveat(int dfd, const char *path, char * const argv[],
			   char * const envp[])
{
	(void)dfd;
	(void)path;
	(void)argv;
	(void)envp;

	__warning("%s: Function not implemented", __func__);
	return __set_errno(ENOSYS, -1);
}

hidden int __ldso_posix_spawn(pid_t *pid,
			      const char *path,
			      const posix_spawn_file_actions_t *file_actions,
			      const posix_spawnattr_t *attrp,
			      char * const argv[],
			      char * const envp[])
{
	(void)pid;
	(void)path;
	(void)file_actions;
	(void)attrp;
	(void)argv;
	(void)envp;

	__warning("%s: Function not implemented", __func__);
	return __set_errno(ENOSYS, -1);
}
#endif

/* New API with addr and addrsiz from mmap() */
static int __elf_header(void *addr, size_t addrsiz, Elf64_Ehdr *ehdr)
{
	if (sizeof(*ehdr) > addrsiz)
		return __set_errno_and_perror(ERANGE, -1);

	memcpy(ehdr, addr, sizeof(*ehdr));

	/* Not an ELF */
	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
		return __set_errno(ENOEXEC, -1);

	/* It is a 32-bit ELF */
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
		Elf32_Ehdr *ehdr32 = (Elf32_Ehdr *)ehdr;

		ehdr32->e_type = __elf32_half(ehdr32, ehdr32->e_type);
		ehdr32->e_machine = __elf32_half(ehdr32, ehdr32->e_machine);
		ehdr32->e_version = __elf32_word(ehdr32, ehdr32->e_version);
		ehdr32->e_entry = __elf32_address(ehdr32, ehdr32->e_entry);
		ehdr32->e_phoff = __elf32_offset(ehdr32, ehdr32->e_phoff);
		ehdr32->e_shoff = __elf32_offset(ehdr32, ehdr32->e_shoff);
		ehdr32->e_flags = __elf32_word(ehdr32, ehdr32->e_flags);
		ehdr32->e_ehsize = __elf32_half(ehdr32, ehdr32->e_ehsize);
		ehdr32->e_phentsize = __elf32_half(ehdr32, ehdr32->e_phentsize);
		ehdr32->e_phnum = __elf32_half(ehdr32, ehdr32->e_phnum);
		ehdr32->e_shentsize = __elf32_half(ehdr32, ehdr32->e_phentsize);
		ehdr32->e_shnum = __elf32_half(ehdr32, ehdr32->e_shnum);
		ehdr32->e_shstrndx = __elf32_half(ehdr32, ehdr32->e_shstrndx);

		return 0;
	}

	/* It is a 64-bit ELF */
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64) {
		Elf64_Ehdr *ehdr64 = ehdr;

		ehdr64->e_type = __elf64_half(ehdr64, ehdr64->e_type);
		ehdr64->e_machine = __elf64_half(ehdr64, ehdr64->e_machine);
		ehdr64->e_version = __elf64_word(ehdr64, ehdr64->e_version);
		ehdr64->e_entry = __elf64_address(ehdr64, ehdr64->e_entry);
		ehdr64->e_phoff = __elf64_offset(ehdr64, ehdr64->e_phoff);
		ehdr64->e_shoff = __elf64_offset(ehdr64, ehdr64->e_shoff);
		ehdr64->e_flags = __elf64_word(ehdr64, ehdr64->e_flags);
		ehdr64->e_ehsize = __elf64_half(ehdr64, ehdr64->e_ehsize);
		ehdr64->e_phentsize = __elf64_half(ehdr64, ehdr64->e_phentsize);
		ehdr64->e_phnum = __elf64_half(ehdr64, ehdr64->e_phnum);
		ehdr64->e_shentsize = __elf64_half(ehdr64, ehdr64->e_phentsize);
		ehdr64->e_shnum = __elf64_half(ehdr64, ehdr64->e_shnum);
		ehdr64->e_shstrndx = __elf64_half(ehdr64, ehdr64->e_shstrndx);

		return 0;
	}

	/* Unsupported yet! */
	return __set_errno(ENOTSUP, -1);
}

static int __elf32_program_header(void *addr, size_t addrsiz, Elf32_Ehdr *ehdr,
				  Elf32_Phdr *phdr, off_t offset)
{
	if ((size_t)offset + sizeof(*phdr) > addrsiz)
		return __set_errno_and_perror(ERANGE, -1);

	memcpy(phdr, addr + offset, sizeof(*phdr));

	if (!__elf32_swap(ehdr))
		return 0;

	phdr->p_type = __bswap_32__(phdr->p_type);
	phdr->p_offset = __bswap_32__(phdr->p_offset);
	phdr->p_vaddr = __bswap_32__(phdr->p_vaddr);
	phdr->p_paddr = __bswap_32__(phdr->p_paddr);
	phdr->p_filesz = __bswap_32__(phdr->p_filesz);
	phdr->p_memsz = __bswap_32__(phdr->p_memsz);
	phdr->p_flags = __bswap_32__(phdr->p_flags);
	phdr->p_align = __bswap_32__(phdr->p_align);

	return 0;
}

static int __elf64_program_header(void *addr, size_t addrsiz, Elf64_Ehdr *ehdr,
				  Elf64_Phdr *phdr, off_t offset)
{
	if ((size_t)offset + sizeof(*phdr) > addrsiz)
		return __set_errno_and_perror(ERANGE, -1);

	memcpy(phdr, addr + offset, sizeof(*phdr));

	if (!__elf64_swap(ehdr))
		return 0;

	phdr->p_type = __bswap_32__(phdr->p_type);
	phdr->p_flags = __bswap_32__(phdr->p_flags);
	phdr->p_offset = __bswap_64__(phdr->p_offset);
	phdr->p_vaddr = __bswap_64__(phdr->p_vaddr);
	phdr->p_paddr = __bswap_64__(phdr->p_paddr);
	phdr->p_filesz = __bswap_64__(phdr->p_filesz);
	phdr->p_memsz = __bswap_64__(phdr->p_memsz);
	phdr->p_align = __bswap_64__(phdr->p_align);

	return 0;
}

static int __elf32_iterate_for_pt(void *addr,
				  size_t addrsiz,
				  Elf32_Ehdr *ehdr,
				  unsigned int p_type,
				  int (*callback)(const void *, size_t, void *),
				  void *data)
{
	const int errno_save = errno;
	int i, num;
	off_t off;

	if (!callback)
		return __set_errno_and_perror(EINVAL, -1);

	/* Look for the segment */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf32_Phdr phdr;
		int err;

		err = __elf32_program_header(addr, addrsiz, ehdr, &phdr, off);
		if (err == -1)
			return -1;

		off += sizeof(phdr);

		/* Not the segment */
		if (phdr.p_type != p_type)
			continue;

		if (phdr.p_offset > addrsiz)
			return __set_errno_and_perror(ERANGE, -1);

		err = callback(addr + phdr.p_offset, phdr.p_filesz, data);
		if (err == -1)
			return -1;

		if (err == 0)
			return __set_errno(errno_save, err);
	}

	return __set_errno(ENOENT, -1);
}

static int __elf64_iterate_for_pt(void *addr,
				  size_t addrsiz,
				  Elf64_Ehdr *ehdr,
				  unsigned int p_type,
				  int (*callback)(const void *, size_t, void *),
				  void *data)
{
	const int errno_save = errno;
	int i, num;
	off_t off;

	if (!callback)
		return __set_errno_and_perror(EINVAL, -1);

	/* Look for the segment */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf64_Phdr phdr;
		int err;

		err = __elf64_program_header(addr, addrsiz, ehdr, &phdr, off);
		if (err == -1)
			return -1;

		off += sizeof(phdr);

		/* Not the segment */
		if (phdr.p_type != p_type)
			continue;

		if (phdr.p_offset > addrsiz)
			return __set_errno_and_perror(ERANGE, -1);

		err = callback(addr + phdr.p_offset, phdr.p_filesz, data);
		if (err == -1)
			return -1;

		if (err == 0)
			return __set_errno(errno_save, err);
	}

	return __set_errno(ENOENT, -1);
}

static int __elf_iterate_shared_object_for_pt(int fd, unsigned int p_type,
		     int (*callback)(const void *, size_t, void *), void *data)
{
	struct stat statbuf;
	Elf64_Ehdr ehdr;
	size_t addrsiz;
	void *addr;
	int err;

	err = next_fstat(fd, &statbuf);
	if (err == -1)
		return -1;

	addrsiz = statbuf.st_size;
	if (addrsiz == 0)
		return __set_errno_and_perror(ERANGE, -1);

	addr = mmap(NULL, addrsiz, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
		return -1;

	err = __elf_header(addr, addrsiz, &ehdr);
	if (err == -1)
		return -1;

	/* Not a linked program or shared object */
	if ((ehdr.e_type != ET_EXEC) && (ehdr.e_type != ET_DYN))
		return __set_errno(ENOEXEC, -1);

	/* It is a 32-bit ELF */
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		return __elf32_iterate_for_pt(addr,
					      addrsiz,
					      (Elf32_Ehdr *)&ehdr,
					      p_type,
					      callback,
					      data);

	/* It is a 64-bit ELF */
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		return __elf64_iterate_for_pt(addr,
					      addrsiz,
					      (Elf64_Ehdr *)&ehdr,
					      p_type,
					      callback,
					      data);

	/* Unsupported yet! */
	return __set_errno(ENOTSUP, -1);
}

static int __elf_has_interp_callback(const void *data, size_t datasiz,
				     void *user)
{
	const char *interp = (const char *)data;
	int *fd = (int *)user;
	(void)datasiz;
	(void)interp;
	(void)fd;

	if (!data || !user)
		return __set_errno_and_perror(EINVAL, -1);

	__notice("%s: has interpreter '%s'\n", __fpath(*fd), interp);

	return 0;
}

hidden int __elf_has_interp(int fd)
{
	const int errno_save = errno;
	int err;

	err = __elf_iterate_shared_object_for_pt(fd,
						 PT_INTERP,
						 __elf_has_interp_callback,
						 &fd);
	if (err == -1 && errno != ENOENT)
		return -1;
	if (err == -1)
		return __set_errno(errno_save, 0);

	return __set_errno(errno_save, 1);
}

#ifdef __NetBSD__
#undef next_fstat
#endif
