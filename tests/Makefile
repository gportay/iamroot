#
# Copyright 2021-2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

OS ?= $(shell uname -o)

override CPPFLAGS += -D_GNU_SOURCE
override CFLAGS += -fPIC -Wall -Wextra

IAMROOT_LIB ?= $(CURDIR)/libiamroot.so
export IAMROOT_LIB

ifdef COVERAGE
test-%: override CFLAGS += -fprofile-arcs -ftest-coverage
test-%: override LDFLAGS += -fprofile-arcs
test-%: override LDLIBS += -lgcov
endif

.PHONY: all
all: libiamnotroot.so
all: test-__xmknod
all: test-__xmknodat
all: test-access
all: test-chmod
all: test-chown
all: test-chroot
all: test-dl_iterate_phdr
all: test-dlopen
all: test-execl
all: test-execle
all: test-execlp
all: test-execv
all: test-execve
all: test-execveat
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
all: test-readlink
all: test-readlinkat
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
all: test-umount
all: test-umount2
all: test-unlink
all: test-unlinkat
all: test-unshare
all: test-utimensat
all: test-utimes

libiamnotroot.so: override LDFLAGS += -nodefaultlibs
libiamnotroot.so: iamnotroot.o

test-dlopen: override LDLIBS += -ldl

test-path_resolution: libiamnotroot.so
test-path_resolution: override LDLIBS += libiamnotroot.so

test-pathsetenv: libiamnotroot.so
test-pathsetenv: override LDLIBS += libiamnotroot.so

.PHONY: test
test: export LD_PRELOAD = $(IAMROOT_LIB)
ifeq ($(OS),FreeBSD)
test: export IAMROOT_LIBRARY_PATH := $(CURDIR):/lib:/usr/lib:/usr/local/lib
endif
ifeq ($(OS),GNU/Linux)
test: export IAMROOT_LIBRARY_PATH := $(CURDIR):/usr/lib64:/lib64
endif
test: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|.*\.gcda
test: export IAMROOT_FATAL = 1
test: | rootfs
	rm -Rf rootfs/tmp/* rootfs/dev/*
	touch rootfs/tmp/file0
	ln -s file0 rootfs/tmp/symlink0
	env PATH=$(CURDIR):$(PATH) test-access rootfs/tmp/file0 0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-access /tmp/file0 0
ifeq ($(OS),FreeBSD)
	env PATH=$(CURDIR):$(PATH) test-chmod rootfs/tmp/file0 00644
else
	env PATH=$(CURDIR):$(PATH) test-chmod rootfs/tmp/file0 01644
endif
	env PATH=$(CURDIR):$(PATH) test-fchmodat . rootfs/tmp/file0 02644
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fchmodat - /tmp/file0 04644
	env PATH=$(CURDIR):$(PATH) test-chown rootfs/tmp/file0 0 0
	env PATH=$(CURDIR):$(PATH) test-faccessat . rootfs/Makefile 0 0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-faccessat - /Makefile 0 0
	env PATH=$(CURDIR):$(PATH) test-fchown rootfs/tmp/file0 0 0
	env PATH=$(CURDIR):$(PATH) test-fchownat - rootfs/tmp/file0 0 0
	env PATH=$(CURDIR):$(PATH) test-stat rootfs/tmp/file0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-stat /tmp/file0
	env PATH=$(CURDIR):$(PATH) test-lstat rootfs/tmp/symlink0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-lstat /tmp/symlink0
	env PATH=$(CURDIR):$(PATH) test-fstat rootfs/tmp/file0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fstat /tmp/file0
	env PATH=$(CURDIR):$(PATH) test-fstatat - rootfs/tmp/file0 0 0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fstatat - /tmp/file0 0 0
	env PATH=$(CURDIR):$(PATH) test-lchown rootfs/tmp/symlink0 0 0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fchownat - /tmp/file0 0 0
	env PATH=$(CURDIR):$(PATH) test-get_current_dir_name | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-get_current_dir_name | tee /dev/stderr | grep -q '^/$$'
	env PATH=$(CURDIR):$(PATH) test-getcwd | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getcwd | tee /dev/stderr | grep -q '^/$$'
	env PATH=$(CURDIR):$(PATH) test-getwd | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getwd | tee /dev/stderr | grep -q '^/$$'
	env PATH=$(CURDIR):$(PATH) test-link rootfs/tmp/file0 rootfs/tmp/link1
	env PATH=$(CURDIR):$(PATH) test-linkat - rootfs/tmp/file0 - rootfs/tmp/link2 0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-linkat - /tmp/file0 - /tmp/link3 0
	env PATH=$(CURDIR):$(PATH) test-mkdir rootfs/tmp/dir0 0555
	env PATH=$(CURDIR):$(PATH) test-mkdirat . rootfs/tmp/dir1 0555
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkdirat . /tmp/dir2 0755
	env PATH=$(CURDIR):$(PATH) test-mknod rootfs/dev/dev0 0644 0 0
	env PATH=$(CURDIR):$(PATH) test-mknodat - rootfs/dev/dev1 0666 0 1
	env PATH=$(CURDIR):$(PATH) test-mknodat . rootfs/dev/dev2 0666 0 2
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknod /dev/dev3 0644 0 3
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknodat - /dev/dev4 0666 0 4
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknodat . /dev/dev5 0666 0 5
	env PATH=$(CURDIR):$(PATH) test-__xmknod 1 rootfs/dev/dev6 0644 0 6
	env PATH=$(CURDIR):$(PATH) test-__xmknodat 1 - rootfs/dev/dev7 0666 0 7
	env PATH=$(CURDIR):$(PATH) test-__xmknodat 1 . rootfs/dev/dev8 0666 0 8
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-__xmknod 1 /dev/dev9 0644 0 9
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-__xmknodat 1 - /dev/dev10 0666 0 10
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-__xmknodat 1 . /dev/dev11 0666 0 11
	env PATH=$(CURDIR):$(PATH) test-mount -t procfs none rootfs/proc
	env PATH=$(CURDIR):$(PATH) test-umount rootfs/proc
	env PATH=$(CURDIR):$(PATH) test-umount2 rootfs/proc
	env PATH=$(CURDIR):$(PATH) LD_LIBRARY_PATH=$(CURDIR) test-path_resolution / | tee /dev/stderr | grep -q "^/\$$"
	env PATH=$(CURDIR):$(PATH) LD_LIBRARY_PATH=$(CURDIR) test-path_resolution /proc/ | tee /dev/stderr | grep -q "^/proc/\$$"
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /sys/ | tee /dev/stderr | grep -q "^/sys/\$$"
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /dev/ | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/dev\$$"
	env PATH=$(CURDIR):$(PATH) LD_LIBRARY_PATH=$(CURDIR) test-pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^/usr/local/lib:/usr/lib:/lib$$"
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/usr/local/lib:$(CURDIR)/rootfs/usr/lib:$(CURDIR)/rootfs/lib$$"
	env PATH=$(CURDIR):$(PATH) test-openat - rootfs/tmp/file0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-openat . /tmp/file0
	env PATH=$(CURDIR):$(PATH) test-opendir rootfs/tmp/dir0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-opendir /tmp/dir0
	env PATH=$(CURDIR):$(PATH) test-rename rootfs/tmp/file0 rootfs/tmp/file1
	env PATH=$(CURDIR):$(PATH) test-renameat - rootfs/tmp/file1 - rootfs/tmp/file2
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-renameat - /tmp/file2 - /tmp/file3
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-renameat2 - /tmp/file3 - /tmp/file0 0
	env PATH=$(CURDIR):$(PATH) test-rmdir rootfs/tmp/dir0
	env PATH=$(CURDIR):$(PATH) test-rmdir rootfs/tmp/dir1
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-rmdir /tmp/dir2
	env PATH=$(CURDIR):$(PATH) test-sanitize ""                  | tee /dev/stderr | grep -vc '^$$'   | grep -q '^0$$'
	env PATH=$(CURDIR):$(PATH) test-sanitize . ./ ././ ./. ././. | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	env PATH=$(CURDIR):$(PATH) test-sanitize / // ///  //. //./. | tee /dev/stderr | grep -vc '^/$$'  | grep -q '^0$$'
	env PATH=$(CURDIR):$(PATH) test-scandir .
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-scandirat - /
	env PATH=$(CURDIR):$(PATH) test-statfs .
	env PATH=$(CURDIR):$(PATH) test-statvfs .
	env PATH=$(CURDIR):$(PATH) test-statx - rootfs/tmp/file0 0 0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-statx - /tmp/file0 0 0
	env PATH=$(CURDIR):$(PATH) test-symlink file0 rootfs/tmp/symlink1
	env PATH=$(CURDIR):$(PATH) test-symlinkat file0 . rootfs/tmp/symlink2
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-symlinkat file0 . /tmp/symlink3
	env PATH=$(CURDIR):$(PATH) test-readlink /proc/self/exe                                                      | tee /dev/stderr | grep -q "^$(CURDIR)/test-readlink$$"
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-readlink /proc/self/exe     | tee /dev/stderr | grep -q "^/usr/bin/test-readlink$$"
	env PATH=$(CURDIR):$(PATH) test-readlinkat . /proc/self/exe                                                  | tee /dev/stderr | grep -q "^$(CURDIR)/test-readlinkat$$"
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-readlinkat . /proc/self/exe | tee /dev/stderr | grep -q "^/usr/bin/test-readlinkat$$"
	env PATH=$(CURDIR):$(PATH) test-utimes rootfs/tmp/file0 0x3fffffff 0x3fffffff
	env PATH=$(CURDIR):$(PATH) test-lutimes rootfs/tmp/symlink0 0x3fffffff 0x3fffffff
	env PATH=$(CURDIR):$(PATH) test-utimensat . rootfs/tmp/file0 0x3fffffff 0x3fffffff
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-utimensat . /tmp/file0 0x3fffffff 0x3fffffff
	env PATH=$(CURDIR):$(PATH) test-unlink rootfs/tmp/file0
	env PATH=$(CURDIR):$(PATH) test-unlinkat . rootfs/dev/dev1 0
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-unlinkat - /dev/dev2 0
	env PATH=$(CURDIR):$(PATH) test-unshare whoami
	env PATH=$(CURDIR):$(PATH) LD_LIBRARY_PATH=$(CURDIR) test-dlopen libiamnotroot.so
	cd rootfs/ && env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs IAMROOT_LIBRARY_PATH=/usr/local/lib:/usr/lib64:/usr/lib:/lib64:/lib test-dlopen libiamnotroot.so
	! test-dl_iterate_phdr | grep '$(CURDIR)/rootfs'
	cd rootfs/ && ! env PATH=/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-dl_iterate_phdr | grep '$(CURDIR)/rootfs'
	env PATH=$(CURDIR):$(PATH) test-execl | grep '^one two three$$'
	env PATH=$(CURDIR):$(PATH) test-execle | grep '^one two three$$'
	env PATH=$(CURDIR):$(PATH) test-execlp | grep '^one two three$$'
	env PATH=$(CURDIR):$(PATH) test-execv | grep '^one two three$$'
	env PATH=$(CURDIR):$(PATH) test-execve | grep '^one two three$$'
	env PATH=$(CURDIR):$(PATH) test-execveat | grep '^one two three$$'
	env PATH=$(CURDIR):$(PATH) test-execvp | grep '^one two three$$'
	env PATH=$(CURDIR):$(PATH) test-execvpe | grep '^one two three$$'
	env PATH=$(CURDIR):$(PATH) test-fexecve | grep '^one two three$$'

rootfs: DESTDIR = $(CURDIR)/rootfs
rootfs:
	install -d -m755 $(DESTDIR)
	install -m644 Makefile $(DESTDIR)/
	install -d -m755 $(DESTDIR)/usr/bin
	for i in *.c; do \
		test -e test-$${i%.c} || continue; \
		install -m755 test-$${i%.c} $(DESTDIR)/usr/bin/; \
	done
ifeq ($(OS),FreeBSD)
	install -d -m755 $(DESTDIR)/libexec $(DESTDIR)/lib $(DESTDIR)/usr/lib $(DESTDIR)/usr/local/lib
	install -m444 /lib/libc.so.7 $(DESTDIR)/lib
	install -m444 /usr/lib/libdl.so.1 $(DESTDIR)/usr/lib
	install -m555 /libexec/ld-elf.so.1 $(DESTDIR)/libexec
	install -m444 libiamnotroot.so $(DESTDIR)/usr/local/lib
endif
ifeq ($(OS),GNU/Linux)
	install -D -m755 /usr/lib64/libc.so.6 /usr/lib64/libdl.so.2 --target-directory=$(DESTDIR)/usr/lib64
	install -D -m755 /lib64/ld-linux-x86-64.so.2 --target-directory=$(DESTDIR)/lib64
	install -D -m755 libiamnotroot.so --target-directory=$(DESTDIR)/usr/lib64
endif
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
