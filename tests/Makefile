#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

override CFLAGS += -fPIC -Wall -Wextra -Werror
override CFLAGS += -D_GNU_SOURCE -Wno-error=deprecated-declarations

.PHONY: all
all: libiamnotroot.so
all: test-access
all: test-chmod
all: test-chown
all: test-chroot
all: test-execl
all: test-execle
all: test-execlp
all: test-execv
all: test-execvp
all: test-execvpe
all: test-faccessat
all: test-fchmodat
all: test-fchown
all: test-fchownat
all: test-fexecve
all: test-fstat
all: test-fstatat
all: test-get_current_dir_name
all: test-getcwd
all: test-getwd
all: test-lchown
all: test-link
all: test-linkat
all: test-lstat
all: test-lutimes
all: test-mkdir
all: test-mkdirat
all: test-mknod
all: test-mknodat
all: test-mount
all: test-openat
all: test-opendir
all: test-path_resolution
all: test-pathsetenv
all: test-rename
all: test-renameat
all: test-renameat2
all: test-rmdir
all: test-sanitize
all: test-scandir
all: test-scandirat
all: test-stat
all: test-statfs
all: test-statvfs
all: test-statx
all: test-symlink
all: test-symlinkat
all: test-unlink
all: test-unlinkat
all: test-unshare
all: test-utimensat
all: test-utimes

libiamnotroot.so: iamnotroot.o
libiamnotroot.so: override LDLIBS += -ldl

test-path_resolution: libiamnotroot.so
test-path_resolution: override LDLIBS += libiamnotroot.so

test-pathsetenv: libiamnotroot.so
test-pathsetenv: override LDLIBS += libiamnotroot.so

.PHONY: test
test: PATH := $(CURDIR):$(PATH)
test: PREFIX = /usr
test: export LD_LIBRARY_PATH := $(CURDIR):$(LD_LIBRARY_PATH)
test: export IAMROOT_LIB_LINUX_X86_64_2 = $(IAMROOT_LIB)
test: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 =
test: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|$(IAMROOT_LIB)|/lib64/ld-linux-x86-64.so.2
test: | rootfs
	rm -Rf rootfs/tmp/* rootfs/dev/*
	touch rootfs/tmp/file0
	ln -s file0 rootfs/tmp/symlink0
	test-access rootfs/tmp/file0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-access /tmp/file0 0
	test-chmod rootfs/tmp/file0 01644
	test-fchmodat . rootfs/tmp/file0 02644
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fchmodat - /tmp/file0 04644
	test-chown rootfs/tmp/file0 0 0
	test-faccessat . rootfs/Makefile 0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-faccessat - /Makefile 0 0
	test-fchown rootfs/tmp/file0 0 0
	test-fchownat - rootfs/tmp/file0 0 0
	test-stat rootfs/tmp/file0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-stat /tmp/file0
	test-lstat rootfs/tmp/symlink0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-lstat /tmp/symlink0
	test-fstat rootfs/tmp/file0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fstat /tmp/file0
	test-fstatat - rootfs/tmp/file0 0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fstatat - /tmp/file0 0 0
	test-lchown rootfs/tmp/symlink0 0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fchownat - /tmp/file0 0 0
	test-get_current_dir_name | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-get_current_dir_name | tee /dev/stderr | grep -q '/'
	test-getcwd | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getcwd | tee /dev/stderr | grep -q '/'
	test-getwd | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getwd | tee /dev/stderr | grep -q '/'
	test-link rootfs/tmp/file0 rootfs/tmp/link1
	test-linkat - rootfs/tmp/file0 - rootfs/tmp/link2 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-linkat - /tmp/file0 - /tmp/link3 0
	test-mkdir rootfs/tmp/dir0 0555
	test-mkdirat . rootfs/tmp/dir1 0555
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkdirat . /tmp/dir2 0755
	test-mknod rootfs/dev/dev0 0644 0 0
	test-mknodat - rootfs/dev/dev1 0666 0 1
	test-mknodat . rootfs/dev/dev2 0666 0 2
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknodat - /dev/dev3 0666 0 3
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknodat . /dev/dev4 0666 0 4
	test-mount -t procfs none rootfs/proc
	test-path_resolution / | tee /dev/stderr | grep -q "^/\$$"
	test-path_resolution /proc/ | tee /dev/stderr | grep -q "^/proc/\$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /sys/ | tee /dev/stderr | grep -q "^/sys/\$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /dev/ | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/dev\$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution "$$IAMROOT_LIB"  | tee /dev/stderr | grep -q "^$$IAMROOT_LIB$$"
	test-pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^/usr/local/lib:/usr/lib:/lib$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/usr/local/lib:$(CURDIR)/rootfs/usr/lib:$(CURDIR)/rootfs/lib$$"
	test-openat - rootfs/tmp/file0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-openat . /tmp/file0
	test-opendir rootfs/tmp/dir0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-opendir /tmp/dir0
	test-rename rootfs/tmp/file0 rootfs/tmp/file1
	test-renameat - rootfs/tmp/file1 - rootfs/tmp/file2
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-renameat - /tmp/file2 - /tmp/file3
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-renameat2 - /tmp/file3 - /tmp/file0 0
	test-rmdir rootfs/tmp/dir0
	test-rmdir rootfs/tmp/dir1
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-rmdir /tmp/dir2
	test-sanitize ""                  | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	test-sanitize . ./ ././ ./. ././. | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	test-sanitize / // ///  //. //./. | tee /dev/stderr | grep -vc '^/$$'  | grep -q '^0$$'
	test-scandir .
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-scandirat - /
	test-statfs .
	test-statvfs .
	test-statx - rootfs/tmp/file0 0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-statx - /tmp/file0 0 0
	test-symlink file0 rootfs/tmp/symlink1
	test-symlinkat file0 . rootfs/tmp/symlink2
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-symlinkat file0 . /tmp/symlink3
	test-utimes rootfs/tmp/file0 0x3fffffff 0x3fffffff
	test-lutimes rootfs/tmp/symlink0 0x3fffffff 0x3fffffff
	test-utimensat . rootfs/tmp/file0 0x3fffffff 0x3fffffff
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-utimensat . /tmp/file0 0x3fffffff 0x3fffffff
	test-unlink rootfs/tmp/file0
	test-unlinkat . rootfs/dev/dev1 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-unlinkat - /dev/dev2 0
	test-unshare whoami

rootfs: DESTDIR = $(CURDIR)/rootfs
rootfs: PREFIX ?= /usr
rootfs: install

.PHONY: install
install:
	install -D -m644 Makefile $(DESTDIR)/Makefile
	install -D -m755 test-* --target-directory=$(DESTDIR)$(PREFIX)/bin
	install -D -m755 libiamnotroot.so --target-directory=$(DESTDIR)$(PREFIX)/lib
	install -d -m755 $(DESTDIR)/proc
	install -d -m755 $(DESTDIR)/sys
	install -d -m755 $(DESTDIR)/dev
	install -d -m777 $(DESTDIR)/tmp

.PHONY: clean
clean:
	rm -f libiamnotroot.so *.o test-*
	rm -Rf rootfs/

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

test-%: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
