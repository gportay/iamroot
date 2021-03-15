/*
 * Copyright 2020 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _PATH_RESOLUTION_H
#define _PATH_RESOLUTION_H

#ifdef __cplusplus
extern "C" {
#endif

char *path_resolution(const char *path, char *buf, size_t bufsize, int flags);

#ifdef __cplusplus
}
#endif

#endif
