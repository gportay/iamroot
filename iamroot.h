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

#define __strlcmp(s1, s2) strncmp(s1, s2, strlen(s2))
#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)
#define __strlcpy(s1, s2) strncpy(s1, s2, strlen(s1))
#define __strncpy(s1, s2) strncpy(s1, s2, sizeof(s1)-1)

char *sanitize(char *, size_t);
char *path_resolution(const char *, char *, size_t, int);
char *fpath_resolutionat(int, const char *, char *, size_t, int);

void __procfdname(char *, unsigned);

const char *getrootdir();
int chrootdir(const char *);
int inchroot();

int __fprintf(FILE *f, const char *fmt, ...) __attribute__ ((format(printf,2,3)));

#define __verbose(fmt, ...) __fprintf(stderr, fmt, __VA_ARGS__)

#define __dl_perror(s) fprintf(stderr, "%s: %s\n", s, dlerror())

#ifdef __cplusplus
}
#endif

#endif
