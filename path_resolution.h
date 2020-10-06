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

const char *path_resolution(const char *path, char *buf, size_t bufsize);

#ifdef __cplusplus
}
#endif

#endif
