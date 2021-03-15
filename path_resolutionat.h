/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _PATH_RESOLUTIONAT_H
#define _PATH_RESOLUTIONAT_H

#ifdef __cplusplus
extern "C" {
#endif

char *path_resolutionat(const char *path, char *buf, size_t bufsize, int flags);

#ifdef __cplusplus
}
#endif

#endif
