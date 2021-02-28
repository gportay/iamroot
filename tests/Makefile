#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

override CFLAGS += -fPIC -Wall -Wextra -Werror

.PHONY: all
all: mount

.PHONY: tests
tests: PATH := $(CURDIR):$(PATH)
tests:
	mount -t procfs none /proc
