#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

override CFLAGS += -fPIC -Wall -Wextra -Werror

TESTS += access
TESTS += chown
TESTS += chroot
TESTS += faccessat
TESTS += fchown
TESTS += fchownat
TESTS += fstatat
TESTS += lchown
TESTS += link
TESTS += linkat
TESTS += lutimes
TESTS += mknod
TESTS += mknodat
TESTS += mount
TESTS += openat
TESTS += sanitize
TESTS += symlink
TESTS += symlinkat
TESTS += unlink
TESTS += unlinkat
TESTS += unshare
TESTS += utimensat
TESTS += utimes

.PHONY: all
all: $(TESTS)

.PHONY: tests
tests: PATH := $(CURDIR):$(PATH)
tests: ALPINE_MINIROOTFS ?= .
tests:
	rm -f dev* file* symlink[0-9] link[0-9]*
	access Makefile 0
	IAMROOT_ROOT=$(CURDIR) /access /Makefile 0
	chroot $(ALPINE_MINIROOTFS) whoami
	chown file 0 0
	faccessat . Makefile 0 0
	IAMROOT_ROOT=$(CURDIR) /faccessat - /Makefile 0 0
	fchown file 0 0
	fchownat - file 0 0
	fstatat - file 0 0
	IAMROOT_ROOT=$(CURDIR) /fstatat - /file 0 0
	lchown symlink0 0 0
	IAMROOT_ROOT=$(CURDIR) /fchownat - /file 0 0
	link file link1
	linkat - file - link2 0
	IAMROOT_ROOT=$(CURDIR) /linkat - /file - /link3 0
	mknod dev 0644 0 0
	mknodat - dev1 0666 0 1
	mknodat . dev2 0666 0 2
	IAMROOT_ROOT=$(CURDIR) /mknodat - /dev3 0666 0 3
	IAMROOT_ROOT=$(CURDIR) /mknodat . /dev4 0666 0 4
	mount -t procfs none /proc
	openat - file
	IAMROOT_ROOT=$(CURDIR) /openat . /file
	sanitize . ./ ././ | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	sanitize / // ///  | tee /dev/stderr | grep -vc '^/$$'  | grep -q '^0$$'
	symlink file symlink1
	symlinkat file . symlink2
	IAMROOT_ROOT=$(CURDIR) /symlinkat file . symlink3
	utimes file 0x3fffffff 0x3fffffff
	lutimes symlink0 0x3fffffff 0x3fffffff
	utimensat . file 0x3fffffff 0x3fffffff
	IAMROOT_ROOT=$(CURDIR) /utimensat . /file 0x3fffffff 0x3fffffff
	unlink file
	unlinkat . dev1 0
	IAMROOT_ROOT=$(CURDIR) /unlinkat - /dev2 0
	unshare whoami

.PHONY: clean
clean:
	rm -f $(TESTS)
