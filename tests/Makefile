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
test: export IAMROOT_USE_HOST_INTERP = 1
test: ALPINE_MINIROOTFS ?= .
test: | rootfs
	rm -Rf rootfs/proc rootfs/sys rootfs/tmp rootfs/dev* rootfs/file* rootfs/symlink[0-9] rootfs/link[0-9]*
	touch rootfs/file
	ln -s file rootfs/symlink0
	test-access rootfs/Makefile 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-access /Makefile 0
	IAMROOT_USE_HOST_INTERP= test-chroot $(ALPINE_MINIROOTFS) whoami
	test-chmod rootfs/file 01644
	test-fchmodat . rootfs/file 02644
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fchmodat - /file 04644
	test-chown rootfs/file 0 0
	test-faccessat . rootfs/Makefile 0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-faccessat - /Makefile 0 0
	test-fchown rootfs/file 0 0
	test-fchownat - rootfs/file 0 0
	test-stat rootfs/file
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-stat /file
	test-lstat rootfs/symlink0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-lstat /symlink0
	test-fstat rootfs/file
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fstat rootfs/file
	test-fstatat - rootfs/file 0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fstatat - /file 0 0
	test-lchown rootfs/symlink0 0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fchownat - /file 0 0
	test-get_current_dir_name | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-get_current_dir_name | tee /dev/stderr | grep -q '/'
	test-getcwd | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getcwd | tee /dev/stderr | grep -q '/'
	test-getwd | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getwd | tee /dev/stderr | grep -q '/'
	test-link rootfs/file rootfs/link1
	test-linkat - rootfs/file - rootfs/link2 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-linkat - /file - /link3 0
	test-mkdir rootfs/proc 0555
	test-mkdirat . rootfs/sys 0555
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkdirat . /tmp 0755
	test-mknod rootfs/dev 0644 0 0
	test-mknodat - rootfs/dev1 0666 0 1
	test-mknodat . rootfs/dev2 0666 0 2
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknodat - /dev3 0666 0 3
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknodat . /dev4 0666 0 4
	test-mount -t procfs none rootfs/proc
	test-path_resolution / | tee /dev/stderr | grep -q "^/\$$"
	test-path_resolution /proc/ | tee /dev/stderr | grep -q "^/proc/\$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /sys/ | tee /dev/stderr | grep -q "^/sys/\$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /dev/ | tee /dev/stderr | grep -q "^/dev/\$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /run/ | tee /dev/stderr | grep -q "^/run/\$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /etc/passwd | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/etc/passwd\$$"
	IAMROOT_PATH_RESOLUTION_IGNORE="^/(proc|sys)/|$$IAMROOT_LIB" \
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution "$$IAMROOT_LIB"  | tee /dev/stderr | grep -q "^$$IAMROOT_LIB$$"
	test-pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^/usr/local/lib:/usr/lib:/lib$$"
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/usr/local/lib:$(CURDIR)/rootfs/usr/lib:$(CURDIR)/rootfs/lib$$"
	test-openat - rootfs/file
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-openat . /file
	test-opendir rootfs/proc
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-opendir /proc
	test-rename rootfs/file rootfs/file1
	test-renameat - rootfs/file1 - rootfs/file2
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-renameat - /file2 - /file3
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-renameat2 - /file3 - /file 0
	test-rmdir rootfs/proc
	test-rmdir rootfs/sys
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-rmdir /tmp
	test-sanitize ""                  | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	test-sanitize . ./ ././ ./. ././. | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	test-sanitize / // ///  //. //./. | tee /dev/stderr | grep -vc '^/$$'  | grep -q '^0$$'
	test-scandir .
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-scandirat - /
	test-statfs .
	test-statvfs .
	test-statx - rootfs/file 0 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-statx - /file 0 0
	test-symlink file rootfs/symlink1
	test-symlinkat file . rootfs/symlink2
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-symlinkat file . /symlink3
	test-utimes rootfs/file 0x3fffffff 0x3fffffff
	test-lutimes rootfs/symlink0 0x3fffffff 0x3fffffff
	test-utimensat . rootfs/file 0x3fffffff 0x3fffffff
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-utimensat . /file 0x3fffffff 0x3fffffff
	test-unlink rootfs/file
	test-unlinkat . rootfs/dev1 0
	PATH=$(PREFIX)/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-unlinkat - /dev2 0
	test-unshare whoami

rootfs: DESTDIR = $(CURDIR)/rootfs
rootfs: PREFIX ?= /usr
rootfs: install

.PHONY: install
install:
	install -D -m644 Makefile $(DESTDIR)/Makefile
	install -D -m755 test-* --target-directory=$(DESTDIR)$(PREFIX)/bin
	install -D -m755 libiamnotroot.so --target-directory=$(DESTDIR)$(PREFIX)/lib

.PHONY: clean
clean:
	rm -f libiamnotroot.so *.o test-*
	rm -R rootfs/

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

test-%: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
