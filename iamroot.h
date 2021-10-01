/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _IAMROOT_H_
#define _IAMROOT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern char *path_resolution(const char *, char *, size_t, int);
extern char *fpath_resolutionat(int, const char *, char *, size_t, int);

extern const char *getrootdir();
extern int chrootdir(const char *);
extern int inchroot();

extern int __fprintf(FILE *f, const char *fmt, ...) __attribute__ ((format(printf,2,3)));

#define __verbose(fmt, ...) __fprintf(stderr, fmt, __VA_ARGS__)

#define __dl_perror(s) fprintf(stderr, "%s: %s\n", s, dlerror())

#ifdef __cplusplus
}
#endif

#endif
