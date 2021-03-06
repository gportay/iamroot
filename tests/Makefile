#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

override CFLAGS += -fPIC -Wall -Wextra -Werror

.PHONY: all
all: fchown
all: mount
all: unshare

.PHONY: tests
tests: PATH := $(CURDIR):$(PATH)
tests:
	rm -f file
	fchown file 0 0
	mount -t procfs none /proc
	unshare whoami
