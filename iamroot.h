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

extern int __fprintf(FILE *f, const char *fmt, ...) __attribute__ ((format(printf,2,3)));

#define __verbose(fmt, ...) __fprintf(stderr, fmt, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
