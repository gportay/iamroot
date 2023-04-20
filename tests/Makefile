#
# Copyright 2021-2023 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

.SHELL = bash
.SHELLFLAGS = -ec -o pipefail

ARCH ?= $(shell uname -m)
OS ?= $(shell uname -o)

override CPPFLAGS += -D_GNU_SOURCE
override CFLAGS += -fPIC -Wall -Wextra

LANG ?= en_US.UTF-8
export LANG

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
all: test-acct
all: test-catopen
all: test-chmod
all: test-chown
all: test-chroot
all: test-dl_iterate_phdr
all: test-dlmopen
all: test-dlopen
all: test-endgrent
all: test-endpwent
all: test-endspent
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
all: test-fgetgrent
all: test-fgetpwent
all: test-fgetspent
all: test-fopen
all: test-freopen
all: test-fstat
all: test-fstatat
all: test-ftok
all: test-get_current_dir_name
all: test-getcwd
all: test-getegid
all: test-geteuid
all: test-getgid
all: test-getgrent
all: test-getgrgid
all: test-getgrgid_r
all: test-getgrnam
all: test-getgrnam_r
all: test-getgrouplist
all: test-getgroups
all: test-getgroups
all: test-getpwent
all: test-getpwnam
all: test-getpwnam_r
all: test-getpwuid
all: test-getpwuid_r
all: test-getspnam
all: test-getspnam_r
all: test-getuid
all: test-getwd
all: test-initgroups
all: test-lchown
all: test-lckpwdf
all: test-link
all: test-linkat
all: test-lstat
all: test-lutimes
all: test-mkdir
all: test-mkdirat
all: test-mkdtemp
all: test-mkfifo
all: test-mkfifoat
all: test-mknod
all: test-mknodat
all: test-mkostemp
all: test-mkostemps
all: test-mkostempsat
all: test-mkstemp
all: test-mkstemps
all: test-mktemp
all: test-mount
all: test-name_to_handle_at
all: test-openat
all: test-opendir
all: test-path_resolution
all: test-pathsetenv
all: test-putgrent
all: test-putpwent
all: test-putspent
all: test-readlink
all: test-readlinkat
all: test-rename
all: test-renameat
all: test-renameat2
all: test-rmdir
all: test-sanitize
all: test-scandir
all: test-scandirat
all: test-setegid
all: test-seteuid
all: test-setgid
all: test-setgrent
all: test-setgroups
all: test-setpwent
all: test-setregid
all: test-setresgid
all: test-setresuid
all: test-setreuid
all: test-setspent
all: test-setuid
all: test-stat
all: test-statfs
all: test-statvfs
all: test-statx
all: test-swapoff
all: test-swapon
all: test-symlink
all: test-symlinkat
all: test-tmpnam
all: test-tmpnam_r
all: test-truncate
all: test-truncate64
all: test-ulckpwdf
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
ifeq ($(OS),FreeBSD)
test: export IAMROOT_LIBRARY_PATH := /lib:/usr/lib:/usr/local/lib
test: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/rootfs/lib/libc.so.7:$(CURDIR)/rootfs/usr/lib/libdl.so.1
endif
ifeq ($(OS),GNU/Linux)
test: export IAMROOT_LIBRARY_PATH := /lib64:/usr/lib64
test: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/rootfs/usr/lib64/libc.so.6:$(CURDIR)/rootfs/usr/lib64/libdl.so.2
endif
test: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|.*\.gcda
test: export NLSPATH = /usr/lib/nls/msg/%L/%N
test: | rootfs
	rm -Rf rootfs/tmp/* rootfs/dev/*
	touch rootfs/tmp/file0
	ln -s file0 rootfs/tmp/symlink0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-access rootfs/tmp/file0 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-access /tmp/file0 0
	-env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-acct rootfs/var/log/pacct
	-cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-acct /var/log/pacct
ifeq ($(OS),FreeBSD)
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-chmod rootfs/tmp/file0 00644
endif
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-chmod rootfs/tmp/file0 01644
endif
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-fchmodat . rootfs/tmp/file0 02644
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fchmodat - /tmp/file0 04644
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-chown rootfs/tmp/file0 0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-faccessat . rootfs/Makefile 0 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-faccessat - /Makefile 0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-fchown rootfs/tmp/file0 0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-fchownat - rootfs/tmp/file0 0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-stat rootfs/tmp/file0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-stat /tmp/file0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-lstat rootfs/tmp/symlink0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-lstat /tmp/symlink0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-fstat rootfs/tmp/file0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fstat /tmp/file0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-fstatat - rootfs/tmp/file0 0 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fstatat - /tmp/file0 0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-lchown rootfs/tmp/symlink0 0 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fchownat - /tmp/file0 0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-get_current_dir_name                                             | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-get_current_dir_name | tee /dev/stderr | grep -q '^/$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-getcwd                                                           | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getcwd               | tee /dev/stderr | grep -q '^/$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-getwd                                                            | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getwd                | tee /dev/stderr | grep -q '^/$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-link rootfs/tmp/file0 rootfs/tmp/link1
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-linkat - rootfs/tmp/file0 - rootfs/tmp/link2 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-linkat - /tmp/file0 - /tmp/link3 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkdir rootfs/tmp/dir0 0555
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkdirat . rootfs/tmp/dir1 0555
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkdirat . /tmp/dir2 0755
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkfifo rootfs/tmp/fifo0 0555
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkfifoat . rootfs/tmp/fifo1 0555
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkfifoat . /tmp/fifo2 0755
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mknod rootfs/dev/dev0 0644 0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mknodat - rootfs/dev/dev1 0666 0 1
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mknodat . rootfs/dev/dev2 0666 0 2
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknod /dev/dev3 0644 0 3
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknodat - /dev/dev4 0666 0 4
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mknodat . /dev/dev5 0666 0 5
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-__xmknod 1 rootfs/dev/dev6 0644 0 6
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-__xmknodat 1 - rootfs/dev/dev7 0666 0 7
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-__xmknodat 1 . rootfs/dev/dev8 0666 0 8
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-__xmknod 1 /dev/dev9 0644 0 9
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-__xmknodat 1 - /dev/dev10 0666 0 10
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-__xmknodat 1 . /dev/dev11 0666 0 11
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkostemp rootfs/tmp/tmp-XXXXXX 0                                                | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkostemp /tmp/tmp-XXXXXX 0          | tee /dev/stderr | grep -q "^/tmp/tmp-......\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkostemps rootfs/tmp/tmp-XXXXXXXXX 3 0                                          | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......XXX\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkostemps /tmp/tmp-XXXXXXXXX 3 0    | tee /dev/stderr | grep -q "^/tmp/tmp-......XXX\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkostempsat . rootfs/tmp/tmp-XXXXXXXX 2 0                                       | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......XX\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkostempsat - /tmp/tmp-XXXXXXXX 2 0 | tee /dev/stderr | grep -q "^/tmp/tmp-......XX\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkstemp rootfs/tmp/tmp-XXXXXX                                                   | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkstemp /tmp/tmp-XXXXXX             | tee /dev/stderr | grep -q "^/tmp/tmp-......\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkstemps rootfs/tmp/tmp-XXXXXXXXX 3                                             | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......XXX\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkstemps /tmp/tmp-XXXXXXXXX 3       | tee /dev/stderr | grep -q "^/tmp/tmp-......XXX\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mktemp rootfs/tmp/tmp-XXXXXX                                                    | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mktemp /tmp/tmp-XXXXXX              | tee /dev/stderr | grep -q "^/tmp/tmp-......\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mkdtemp rootfs/tmp/tmpdir-XXXXXX                                                | tee /dev/stderr | grep -q "^rootfs/tmp/tmpdir-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-mkdtemp /tmp/tmpdir-XXXXXX          | tee /dev/stderr | grep -q "^/tmp/tmpdir-......\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-mount -t procfs none rootfs/proc
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-umount rootfs/proc
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-umount2 rootfs/proc
ifeq ($(OS),FreeBSD)
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-path_resolution /                                                                         | tee /dev/stderr | grep -q "^/\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-path_resolution /run/                                                                     | tee /dev/stderr | grep -q "^/run\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-path_resolution /var/run/                                                                 | tee /dev/stderr | grep -q "^/var/run\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/local/lib IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /dev/    | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/dev\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/local/lib IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /run     | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/run\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/local/lib IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /var/run | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/var/run\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-pathsetenv LD_LIBRARY_PATH /lib:/usr/local/lib:/usr/lib                                                                    | tee /dev/stderr | grep -q "^/lib:/usr/local/lib:/usr/lib$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/local/lib IAMROOT_ROOT=$(CURDIR)/rootfs test-pathsetenv LD_LIBRARY_PATH /lib:/usr/local/lib:/usr/lib   | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/lib:$(CURDIR)/rootfs/usr/local/lib:$(CURDIR)/rootfs/usr/lib$$"
endif
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-path_resolution /                                                                         | tee /dev/stderr | grep -q "^/\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-path_resolution /run/                                                                     | tee /dev/stderr | grep -q "^/run\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-path_resolution /var/run/                                                                 | tee /dev/stderr | grep -q "^/run\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-path_resolution /proc/                                                                    | tee /dev/stderr | grep -q "^/proc/\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib64 IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /dev/        | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/dev\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib64 IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /run         | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/run\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib64 IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /var/run     | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/run\$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib64 IAMROOT_ROOT=$(CURDIR)/rootfs test-path_resolution /sys/        | tee /dev/stderr | grep -q "^/sys/\$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-pathsetenv LD_LIBRARY_PATH /lib64:/usr/local/lib64:/usr/lib64                                                              | tee /dev/stderr | grep -q "^/lib64:/usr/local/lib64:/usr/lib64$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib64 IAMROOT_ROOT=$(CURDIR)/rootfs test-pathsetenv LD_LIBRARY_PATH /lib64:/usr/local/lib64:/usr/lib64 | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/lib64:$(CURDIR)/rootfs/usr/local/lib64:$(CURDIR)/rootfs/usr/lib64$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-name_to_handle_at . rootfs/tmp/file0 0
endif
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-name_to_handle_at - /tmp/file0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-ftok rootfs/usr/bin/test-ftok 1
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-ftok /usr/bin/test-ftok 1
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-openat - rootfs/tmp/file0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-openat . /tmp/file0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-opendir rootfs/tmp/dir0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-opendir /tmp/dir0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-fopen rootfs/etc/passwd r
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-fopen /etc/passwd r
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-freopen rootfs/etc/group r
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-freopen /etc/group r
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-catopen "rootfs/usr/lib/nls/msg/$$LANG/null.cat" 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-catopen "/usr/lib/nls/msg/$$LANG/null.cat" 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-catopen null.cat 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-catopen null.cat 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-rename rootfs/tmp/file0 rootfs/tmp/file1
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-renameat - rootfs/tmp/file1 - rootfs/tmp/file2
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-renameat - /tmp/file2 - /tmp/file3
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-renameat2 - /tmp/file3 - /tmp/file0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-rmdir rootfs/tmp/dir0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-rmdir rootfs/tmp/dir1
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-rmdir /tmp/dir2
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-sanitize ""                  | tee /dev/stderr | grep -c '^$$'   | grep -q '^1$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-sanitize . ./ ././ ./. ././. | tee /dev/stderr | grep -c '^\.$$' | grep -q '^5$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-sanitize / // ///  //. //./. | tee /dev/stderr | grep -c '^/$$'  | grep -q '^5$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-scandir .
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-scandirat - /
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-statfs .
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-statvfs .
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-statx - rootfs/tmp/file0 0 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-statx - /tmp/file0 0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-swapon /tmp/file0 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-swapon /tmp/file0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-swapoff /tmp/file0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-swapoff /tmp/file0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-symlink file0 rootfs/tmp/symlink1
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-symlinkat file0 . rootfs/tmp/symlink2
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-symlinkat file0 . /tmp/symlink3
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-readlink /proc/self/exe                                                 | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/usr/bin/test-readlink$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-readlink /proc/self/exe     | tee /dev/stderr | grep -q "^/usr/bin/test-readlink$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-readlinkat . /proc/self/exe                                             | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/usr/bin/test-readlinkat$$"
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-readlinkat . /proc/self/exe | tee /dev/stderr | grep -q "^/usr/bin/test-readlinkat$$"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-tmpnam /tmp/tmp0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-tmpnam /tmp/tmp1
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-tmpnam_r /tmp/tmp2
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-tmpnam_r /tmp/tmp3
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-truncate rootfs/tmp/file0 1
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-truncate /tmp/file0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-truncate64 rootfs/tmp/file0 1
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-truncate64 /tmp/file0 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-utimes rootfs/tmp/file0 0x3fffffff 0x3fffffff
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-lutimes rootfs/tmp/symlink0 0x3fffffff 0x3fffffff
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-utimensat . rootfs/tmp/file0 0x3fffffff 0x3fffffff
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-utimensat . /tmp/file0 0x3fffffff 0x3fffffff
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-unlink rootfs/tmp/file0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-unlinkat . rootfs/dev/dev1 0
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-unlinkat - /dev/dev2 0
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/usr/bin test-unshare whoami
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-dlopen libiamnotroot.so
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-dlopen libiamnotroot.so
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-dlmopen libiamnotroot.so
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-dlmopen libiamnotroot.so
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR) test-dlmopen rootfs/usr/lib64/libiamnotroot.so
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-dlmopen /usr/lib64/libiamnotroot.so
endif
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin test-dl_iterate_phdr                                               | grep '$(CURDIR)/rootfs'
	cd rootfs/ && ! env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-dl_iterate_phdr | grep '$(CURDIR)/rootfs'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin                   test-getgid  | grep '^0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_GID=1000  test-getgid  | grep '^1000$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin                   test-getegid | grep '^0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_EGID=1000 test-getegid | grep '^1000$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setgid     0              | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setegid    0              | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setregid   0  0           | tee /dev/stderr | grep '^0:0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setregid   0 -1           | tee /dev/stderr | grep '^0:$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setregid  -1  0           | tee /dev/stderr | grep '^:0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setresgid  0  0  0        | tee /dev/stderr | grep '^0:0:0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setresgid  0  0 -1        | tee /dev/stderr | grep '^0:0:$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setresgid  0 -1  0        | tee /dev/stderr | grep '^0::0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setresgid -1  0  0        | tee /dev/stderr | grep '^:0:0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin                   test-getuid  | grep '^0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_UID=1000  test-getuid  | grep '^1000$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin                   test-geteuid | grep '^0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_EUID=1000 test-geteuid | grep '^1000$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setuid     0              | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-seteuid    0              | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setreuid   0  0           | tee /dev/stderr | grep '^0:0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setreuid   0 -1           | tee /dev/stderr | grep '^0:$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setreuid  -1 -1           | tee /dev/stderr | grep '^:$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setresuid  0  0  0        | tee /dev/stderr | grep '^0:0:0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setresuid  0  0 -1        | tee /dev/stderr | grep '^0:0:$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setresuid  0 -1  0        | tee /dev/stderr | grep '^0::0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-setresuid -1  0  0        | tee /dev/stderr | grep '^:0:0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-execl    | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-execle   | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-execlp   | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-execv    | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-execve   | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-execveat | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-execvp   | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-execvpe  | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin:/bin test-fexecve  | grep '^one two three$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getpwnam root                                                                    | tee /dev/stderr | grep -q '^root:.*:0:0:.*:.*:.*$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getpwnam_r root                                                                  | tee /dev/stderr | grep -q '^root:.*:0:0:.*:.*:.*$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getpwuid 0                                                                       | tee /dev/stderr | grep -q '^root:.*:0:0:.*:.*:.*$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getpwuid_r 0                                                                     | tee /dev/stderr | grep -q '^root:.*:0:0:.*:.*:.*$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-fgetpwent <passwd
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getpwent
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-setpwent
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-putpwent iamroot x 1000 1000 "iamroot user" /home/iamroot /usr/bin/iamroot-shell | tee /dev/stderr | grep -q '^iamroot:x:1000:1000:iamroot user:/home/iamroot:/usr/bin/iamroot-shell$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-endpwent
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getgrnam root                                                                    | tee /dev/stderr | grep -q '^root:.*:0:.*$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getgrnam_r root                                                                  | tee /dev/stderr | grep -q '^root:.*:0:.*$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getgrgid 0                                                                       | tee /dev/stderr | grep -q '^root:.*:0:.*$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getgrgid_r 0                                                                     | tee /dev/stderr | grep -q '^root:.*:0:.*$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-fgetgrent <group
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getgrent
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-setgrent
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-putgrent iamroot x 1000                                                          | tee /dev/stderr | grep -q '^iamroot:x:1000:$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-putgrent iamroot x 1000 iamroot                                                  | tee /dev/stderr | grep -q '^iamroot:x:1000:iamroot$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-putgrent iamroot x 1000 iamroot wheel                                            | tee /dev/stderr | grep -q '^iamroot:x:1000:iamroot,wheel$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-endgrent
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getgroups                                                                        | tee /dev/stderr | grep -q '^1 0$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin   IAMROOT_GROUPS=0:100:1000 test-getgroups                                                                        | tee /dev/stderr | grep -q '^3 0 100 1000$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin   IAMROOT_GROUPS=0:100:1000 test-getgroups 3                                                                      | tee /dev/stderr | grep -q '^3 0 100 1000$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-initgroups   "$$USER" "$$GROUPS"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getgrouplist "$$USER" "$$GROUPS"
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-getgrouplist "$$USER" "$$GROUPS" 0
	! env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_GROUPS=0:100:1000 test-getgroups 2
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin   IAMROOT_GROUPS=0:100:1000 test-setgroups                                                                        | tee /dev/stderr | grep -q '^$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-setgroups 0 100 1000                                                             | tee /dev/stderr | grep -q '^0:100:1000$$'
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getspnam root                                                    | tee /dev/stderr | grep -q 'root:\*:14871::::::$$'
	cd rootfs/ && env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs test-getspnam_r root                                                  | tee /dev/stderr | grep -q '^root:\*:14871::::::$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-fgetspent <shadow
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-setspent
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-putspent root '*' '14871' -1 -1 -1 -1 -1 -1                                      | tee /dev/stderr | grep -q '^root:\*:14871::::::$$'
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-endspent
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-lckpwdf
	env LD_PRELOAD=$(IAMROOT_LIB) PATH=$(CURDIR)/rootfs/usr/bin                             test-ulckpwdf

rootfs: DESTDIR = $(CURDIR)/rootfs
rootfs:
	install -d -m755 $(DESTDIR)
	install -m644 Makefile $(DESTDIR)/
	install -d -m 755 $(DESTDIR)/etc
	install -m644 passwd $(DESTDIR)/etc/
	install -m644 group  $(DESTDIR)/etc/
	install -m600 shadow $(DESTDIR)/etc/
	install -d -m755 $(DESTDIR)/usr/lib/nls/msg/$$LANG/
	gencat $(DESTDIR)/usr/lib/nls/msg/$$LANG/null.cat /dev/null
	install -d -m755 $(DESTDIR)/usr/bin
	for i in *.c; do \
		test -e $${i%.c} || continue; \
		install -m755 $${i%.c} $(DESTDIR)/usr/bin/; \
	done
ifeq ($(OS),FreeBSD)
	install -d -m755 $(DESTDIR)/libexec $(DESTDIR)/lib $(DESTDIR)/usr/lib $(DESTDIR)/usr/local/lib
	install -m444 /lib/libc.so.7 $(DESTDIR)/lib
	install -m444 /usr/lib/libdl.so.1 $(DESTDIR)/usr/lib
	install -m555 /libexec/ld-elf.so.1 $(DESTDIR)/libexec
	install -m444 libiamnotroot.so $(DESTDIR)/usr/local/lib
endif
ifeq ($(OS),GNU/Linux)
ifeq ($(shell test -d /lib/$(ARCH)-linux-gnu/ && echo 1),1)
	install -D -m755 /usr/lib/$(ARCH)-linux-gnu/libc.so.6 /usr/lib/$(ARCH)-linux-gnu/libdl.so.2 --target-directory=$(DESTDIR)/usr/lib64
	ln -sf lib64 $(DESTDIR)/usr/lib
	ln -sf . $(DESTDIR)/usr/lib/$(ARCH)-linux-gnu
else
	install -D -m755 /usr/lib64/libc.so.6 /usr/lib64/libdl.so.2 --target-directory=$(DESTDIR)/usr/lib64
	ln -sf lib64 $(DESTDIR)/usr/lib
endif
ifeq ($(ARCH),x86_64)
	install -D -m755 /lib64/ld-linux-x86-64.so.2 --target-directory=$(DESTDIR)/lib64
	ln -sf lib64 $(DESTDIR)/lib
endif
ifeq ($(ARCH),aarch64)
	install -D -m755 /lib/ld-linux-aarch64.so.1 --target-directory=$(DESTDIR)/lib
	ln -sf lib $(DESTDIR)/lib64
endif
	install -D -m755 libiamnotroot.so --target-directory=$(DESTDIR)/usr/lib64
endif
	install -d -m755 $(DESTDIR)/proc
	install -d -m755 $(DESTDIR)/sys
	install -d -m755 $(DESTDIR)/dev
	install -d -m777 $(DESTDIR)/tmp
	install -d -m755 $(DESTDIR)/run
	install -d -m755 $(DESTDIR)/var
	install -d -m755 $(DESTDIR)/var/log
ifeq ($(OS),FreeBSD)
	install -d -m755 $(DESTDIR)/var/run
endif
ifeq ($(OS),GNU/Linux)
	ln -sf /run $(DESTDIR)/var/run
endif

.PHONY: clean
clean:
	rm -f *.gcda *.gcno
	rm -f libiamnotroot.so *.o
	for i in *.c; do \
		rm -f $${i%.c}; \
	done
	rm -Rf rootfs/

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
