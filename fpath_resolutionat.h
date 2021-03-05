/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _FPATH_RESOLUTIONAT_H
#define _FPATH_RESOLUTIONAT_H

#ifdef __cplusplus
extern "C" {
#endif

const char *fpath_resolutionat(int fd, const char *path, char *buf,
			       size_t bufsize, int flags);

#ifdef __cplusplus
}
#endif

#endif
