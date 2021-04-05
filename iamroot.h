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

extern int __verbosef(const char *, const char *, ...) __attribute__ ((format(printf,2,3)));

#define __verbose(fmt, ...) __verbosef(__func__, fmt, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
