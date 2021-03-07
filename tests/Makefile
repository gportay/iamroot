#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

override CFLAGS += -fPIC -Wall -Wextra -Werror

.PHONY: all
all: chroot
all: fchown
all: fchownat
all: mknod
all: mknodat
all: mount
all: openat
all: unshare

.PHONY: tests
tests: PATH := $(CURDIR):$(PATH)
tests: ALPINE_MINIROOTFS ?= .
tests:
	rm -f file
	chroot $(ALPINE_MINIROOTFS) whoami
	fchown file 0 0
	fchownat - file 0 0
	IAMROOT_ROOT=$(CURDIR) /fchownat - /file 0 0
	mknod dev 0644 0 0
	mknodat - dev1 0666 0 1
	mknodat . dev2 0666 0 2
	IAMROOT_ROOT=$(CURDIR) /mknodat - /dev3 0666 0 3
	IAMROOT_ROOT=$(CURDIR) /mknodat . /dev4 0666 0 4
	mount -t procfs none /proc
	openat - file
	IAMROOT_ROOT=$(CURDIR) /openat . /file
	unshare whoami
	rm -f file dev dev1 dev2 dev3 dev4
