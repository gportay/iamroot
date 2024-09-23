#
# Copyright 2021-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

SHELL = bash
.SHELLFLAGS = -ec -o pipefail

ARCH ?= $(shell uname -m 2>/dev/null)
KVER ?= $(shell uname -r 2>/dev/null)
OS ?= $(shell uname -o 2>/dev/null || uname -s 2>/dev/null)
XATTR ?= $(shell command -v getfattr 2>/dev/null || command -v setfattr 2>/dev/null && echo xattr)
COVERAGE ?= 0

override CPPFLAGS += -D_GNU_SOURCE
override CFLAGS += -fPIC -Wall -Wextra

LANG ?= en_US.UTF-8
export LANG

IAMROOT_ORIGIN ?= $(CURDIR)
export IAMROOT_ORIGIN

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
all: test-close_range
all: test-closefrom
all: test-creat
all: test-ctermid
all: test-dl_iterate_phdr
all: test-dladdr
all: test-dladdr1
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
all: test-fgetxattr
all: test-flistxattr
all: test-fopen
all: test-fremovexattr
all: test-freopen
all: test-fsetxattr
all: test-fstat
all: test-fstatat
all: test-ftok
all: test-fts
all: test-ftw
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
all: test-getgroupmembership
all: test-getgroups
all: test-getgroups
all: test-getpwent
all: test-getpwnam
all: test-getpwnam_r
all: test-getpwuid
all: test-getpwuid_r
all: test-getresgid
all: test-getresuid
all: test-getspnam
all: test-getspnam_r
all: test-getuid
all: test-getwd
all: test-getxattr
all: test-initgroups
all: test-lchown
all: test-lckpwdf
all: test-lgetxattr
all: test-link
all: test-linkat
all: test-listxattr
all: test-llistxattr
all: test-lremovexattr
all: test-lsetxattr
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
all: test-nftw
all: test-nftw64
all: test-openat
all: test-opendir
all: test-path_resolution
all: test-path_resolution2
all: test-posix_spawn
all: test-putgrent
all: test-putpwent
all: test-putspent
all: test-readlink
all: test-readlinkat
all: test-removexattr
all: test-rename
all: test-renameat
all: test-renameat2
all: test-rmdir
all: test-sanitize
all: test-scandir
all: test-scandirat
all: test-setegid
all: test-seteuid
all: test-setfsgid
all: test-setfsuid
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
all: test-setxattr
all: test-stat
all: test-statfs
all: test-statvfs
all: test-statx
all: test-swapoff
all: test-swapon
all: test-symlink
all: test-symlinkat
all: test-tempnam
all: test-truncate
all: test-truncate64
all: test-ttyname
all: test-ttyname_r
all: test-ulckpwdf
all: test-umount
all: test-umount2
all: test-unlink
all: test-unlinkat
all: test-unshare
all: test-updwtmp
all: test-updwtmpx
all: test-utimensat
all: test-utimes
all: test-utmpname
all: test-utmpxname

libiamnotroot.so: override LDFLAGS += -nodefaultlibs
libiamnotroot.so: iamnotroot.o

ifneq (,$(findstring $(OS),GNU/Linux Linux FreeBSD))
test-dlopen: override LDLIBS += -ldl
test-dlmopen: override LDLIBS += -ldl
test-dladdr: override LDLIBS += -ldl
endif

test-path_resolution: libiamnotroot.so
test-path_resolution: override LDLIBS += libiamnotroot.so

test-path_resolution2: libiamnotroot.so
test-path_resolution2: override LDLIBS += libiamnotroot.so

.PHONY: test
test: test-library test-frontends
	@echo "Completed!"

.PHONY: test-library
test-library: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|.*\.gcda
test-library: export NLSPATH = /usr/lib/nls/msg/%L/%N
test-library: export LIBRARY = $(IAMROOT_ORIGIN)/libiamroot.so
test-library: | rootfs
	rm -Rf rootfs/tmp/* rootfs/dev/*
	touch rootfs/tmp/file0
	ln -s file0 rootfs/tmp/symlink0
ifneq (,$(findstring $(OS),GNU/Linux Linux FreeBSD))
	env LD_PRELOAD=$(LIBRARY) sh -c "unset LD_PRELOAD; chroot rootfs env"
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-creat                rootfs/tmp/wtmp          00644
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-creat                /tmp/wtmpx               00644
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-access               rootfs/tmp/file0         0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-access               /tmp/file0               0
	-env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                           test-acct                 rootfs/var/log/pacct
	-cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs      test-acct                 /var/log/pacct
ifeq ($(OS),FreeBSD)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-chmod                rootfs/tmp/file0         00644
endif
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-chmod                rootfs/tmp/file0         01644
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-fchmodat           . rootfs/tmp/file0         02644
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fchmodat           - /tmp/file0               04644
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-chown                rootfs/tmp/file0         0 0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-faccessat          . rootfs/Makefile          0 0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-faccessat          - /Makefile                0 0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-fchown               rootfs/tmp/file0         0 0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-fchownat           - rootfs/tmp/file0         0 0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-stat                 rootfs/tmp/file0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-stat                 /tmp/file0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-lstat                rootfs/tmp/symlink0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-lstat                /tmp/symlink0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-fstat                rootfs/tmp/file0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fstat                /tmp/file0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-fstatat            - rootfs/tmp/file0         0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fstatat            - /tmp/file0               0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-lchown               rootfs/tmp/symlink0      0 0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fchownat           - /tmp/file0               0 0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-get_current_dir_name                                                       | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-get_current_dir_name                                                       | tee /dev/stderr | grep -q '^/$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-getcwd                                                                     | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getcwd                                                                     | tee /dev/stderr | grep -q '^/$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-getwd                                                                      | tee /dev/stderr | grep -q '^$(CURDIR)$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getwd                                                                      | tee /dev/stderr | grep -q '^/$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-link                 rootfs/tmp/file0   rootfs/tmp/link1
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-linkat             - rootfs/tmp/file0 - rootfs/tmp/link2 0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-linkat             - /tmp/file0       - /tmp/link3       0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkdir                rootfs/tmp/dir0          0555
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkdirat            . rootfs/tmp/dir1          0555
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mkdirat            . /tmp/dir2                0755
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkfifo               rootfs/tmp/fifo0         0555
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkfifoat           . rootfs/tmp/fifo1         0555
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mkfifoat          . /tmp/fifo2                0755
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mknod                rootfs/dev/dev0          0644 0 0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mknodat            - rootfs/dev/dev1          0666 0 1
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mknodat            . rootfs/dev/dev2          0666 0 2
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mknod                /dev/dev3                0644 0 3
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mknodat            - /dev/dev4                0666 0 4
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mknodat            . /dev/dev5                0666 0 5
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-__xmknod         1   rootfs/dev/dev6          0644 0 6
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-__xmknodat       1 - rootfs/dev/dev7          0666 0 7
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-__xmknodat       1 . rootfs/dev/dev8          0666 0 8
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-__xmknod         1   /dev/dev9                0644 0 9
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-__xmknodat       1 - /dev/dev10               0666 0 10
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-__xmknodat       1 . /dev/dev11               0666 0 11
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkostemp             rootfs/tmp/tmp-XXXXXX    0                            | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mkostemp             /tmp/tmp-XXXXXX          0                            | tee /dev/stderr | grep -q "^/tmp/tmp-......\$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkostemps            rootfs/tmp/tmp-XXXXXXXXX 3 0                          | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......XXX\$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mkostemps            /tmp/tmp-XXXXXXXXX       3 0                          | tee /dev/stderr | grep -q "^/tmp/tmp-......XXX\$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkostempsat        . rootfs/tmp/tmp-XXXXXXXX  2 0                          | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......XX\$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mkostempsat        - /tmp/tmp-XXXXXXXX        2 0                          | tee /dev/stderr | grep -q "^/tmp/tmp-......XX\$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkstemp              rootfs/tmp/tmp-XXXXXX                                 | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mkstemp              /tmp/tmp-XXXXXX                                       | tee /dev/stderr | grep -q "^/tmp/tmp-......\$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkstemps             rootfs/tmp/tmp-XXXXXXXXX 3                            | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......XXX\$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mkstemps             /tmp/tmp-XXXXXXXXX       3                            | tee /dev/stderr | grep -q "^/tmp/tmp-......XXX\$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mktemp               rootfs/tmp/tmp-XXXXXX                                 | tee /dev/stderr | grep -q "^rootfs/tmp/tmp-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mktemp               /tmp/tmp-XXXXXX                                       | tee /dev/stderr | grep -q "^/tmp/tmp-......\$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mkdtemp              rootfs/tmp/tmpdir-XXXXXX                              | tee /dev/stderr | grep -q "^rootfs/tmp/tmpdir-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-mkdtemp              /tmp/tmpdir-XXXXXX                                    | tee /dev/stderr | grep -q "^/tmp/tmpdir-......\$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-utmpname             /tmp/wtmp
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-updwtmp              /tmp/wtmp
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-utmpxname            /tmp/wtmpx
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-updwtmpx             /tmp/wtmpx
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-mount -t procfs none rootfs/proc
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-umount               rootfs/proc
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-umount2              rootfs/proc
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-name_to_handle_at  . rootfs/tmp/file0         0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-name_to_handle_at  - /tmp/file0               0
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-ftok                 rootfs/usr/bin/test-ftok 1
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-fts                  rootfs                   0x10
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fts                  /                        0x10
ifeq ($(OS),GNU/Linux)
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-ftok                 /usr/bin/test-ftok       1
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-ftw                  rootfs/usr/bin
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-ftw                  /usr/bin
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-nftw                 rootfs/usr/bin           d
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-nftw                 /usr/bin p
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-nftw64               rootfs/usr/bin           d
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-nftw64               /usr/bin p
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-openat             - rootfs/tmp/file0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-openat             . /tmp/file0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-opendir              rootfs/tmp/dir0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-opendir              /tmp/dir0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-fopen                rootfs/etc/passwd        r
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fopen                /etc/passwd              r
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-freopen              rootfs/etc/group         r
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-freopen              /etc/group               r
ifneq ($(OS),Linux)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-catopen              "rootfs/usr/lib/nls/msg/$$LANG/test.cat" 0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-catopen              "/usr/lib/nls/msg/$$LANG/test.cat"       0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin NLSPATH=$(CURDIR)/rootfs/usr/lib/nls/msg/%L/%N                                             test-catopen              test.cat                                 0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-catopen              test.cat                                 0
endif
ifneq (,$(findstring $(OS),GNU/Linux Linux FreeBSD))
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-close_range 3 3 0                                                     2>&1 | tee /dev/stderr | grep -q '^stdout$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-close_range 3 3 0                                                     2>&1 | tee /dev/stderr | grep -q '^stderr$$' || true
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-close_range 2 3 0                                                     2>&1 | tee /dev/stderr | grep -q '^stdout$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-close_range 2 3 0                                                     2>&1 | tee /dev/stderr | grep -q '^stderr$$' || true
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-close_range 1 3 0                                                     2>&1 | tee /dev/stderr | grep -q '^stderr$$' || true
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-close_range 1 3 0                                                     2>&1 | tee /dev/stderr | grep -q '^stdout$$' || true
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-closefrom 3                                                           2>&1 | tee /dev/stderr | grep -q '^stdout$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-closefrom 3                                                           2>&1 | tee /dev/stderr | grep -q '^stderr$$' || true
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-closefrom 2                                                           2>&1 | tee /dev/stderr | grep -q '^stdout$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-closefrom 2                                                           2>&1 | tee /dev/stderr | grep -q '^stderr$$' || true
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-closefrom 1                                                           2>&1 | tee /dev/stderr | grep -q '^stderr$$' || true
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-closefrom 1                                                           2>&1 | tee /dev/stderr | grep -q '^stdout$$' || true
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-rename               rootfs/tmp/file0   rootfs/tmp/file1
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-renameat           - rootfs/tmp/file1 - rootfs/tmp/file2
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-renameat           - /tmp/file2       - /tmp/file3
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-renameat2          - /tmp/file3       - /tmp/file0            0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-rmdir                rootfs/tmp/dir0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-rmdir                rootfs/tmp/dir1
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs                                                test-rmdir                /tmp/dir2
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-sanitize             ""                                                    | tee /dev/stderr | grep -c '^$$'   | grep -q '^1$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-sanitize             . ./ ././ ./. ././.                                   | tee /dev/stderr | grep -c '^\.$$' | grep -q '^5$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-sanitize             / // ///  //. //./.                                   | tee /dev/stderr | grep -c '^/$$'  | grep -q '^5$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-sanitize             "./this is a very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very path not-exceeding PATH_MAX starting by dot slash"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-scandir              .
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-scandirat            - /
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-statfs               .
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-statvfs              .
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-statx              - rootfs/tmp/file0         0 0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-statx              - /tmp/file0               0 0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-swapon               /tmp/file0               0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-swapon               /tmp/file0               0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-swapoff              /tmp/file0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-swapoff              /tmp/file0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-symlink              file0   rootfs/tmp/symlink1
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-symlinkat            file0 . rootfs/tmp/symlink2
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-symlinkat            file0 . /tmp/symlink3
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-readlink             /proc/self/exe                                        | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/usr/bin/test-readlink$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-readlink             /proc/self/exe                                        | tee /dev/stderr | grep -q "^/usr/bin/test-readlink$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-readlinkat         . /proc/self/exe                                        | tee /dev/stderr | grep -q "^$(CURDIR)/rootfs/usr/bin/test-readlinkat$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-readlinkat         . /proc/self/exe                                        | tee /dev/stderr | grep -q "^/usr/bin/test-readlinkat$$"
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-tempnam                                                                    | tee /dev/stderr | grep -qE "^/tmp/(file|tmp\.|tmp\.....)......$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-tempnam              /run                                                  | tee /dev/stderr | grep -qE "^/run/(file|tmp\.|tmp\.....)......$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-tempnam              /run temp                                             | tee /dev/stderr | grep -qE "^/run/(temp|temp....)......$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-tempnam                                                                    | tee /dev/stderr | grep -qE "^/tmp/(file|tmp\.|tmp\.....)......$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-tempnam              /run                                                  | tee /dev/stderr | grep -qE "^/run/(file|tmp\.|tmp\.....)......$$"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-tempnam              /run temp                                             | tee /dev/stderr | grep -qE "^/run/(temp|temp....)......$$"
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-truncate             rootfs/tmp/file0         1
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-truncate             /tmp/file0               0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-truncate64           rootfs/tmp/file0         1
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-truncate64           /tmp/file0               0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-ctermid
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-ctermid
ifeq ($(shell test -t 0 && echo 1),1)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-ttyname 0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-ttyname 0
endif
ifeq ($(shell test -t 2 && echo 1),1)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-ttyname_r 2
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-ttyname_r 2
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-utimes               rootfs/tmp/file0         0x3fffffff 0x3fffffff
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-lutimes              rootfs/tmp/symlink0      0x3fffffff 0x3fffffff
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-utimensat          . rootfs/tmp/file0         0x3fffffff 0x3fffffff
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-utimensat          . /tmp/file0               0x3fffffff 0x3fffffff
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-unlink               rootfs/tmp/file0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                            test-unlinkat           . rootfs/dev/dev1          0
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-unlinkat           - /dev/dev2                0
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/usr/bin                                                                                   test-unshare              whoami
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs ld_library_path=$(CURDIR)/rootfs/usr/lib                test-dlopen
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs ld_library_path=$(CURDIR)/rootfs/usr/lib                test-dlopen               libiamnotroot.so
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib:/usr/local/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs ld_library_path=$(CURDIR)/rootfs/usr/lib test-dlopen               libiamnotroot.so
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)                                                                                                                    test-dlmopen
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)                                                                                                                    test-dlmopen              libiamnotroot.so
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs ld_library_path=$(CURDIR)/rootfs/usr/lib                test-dlmopen              libiamnotroot.so
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib:/usr/local/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs ld_library_path=$(CURDIR)/rootfs/usr/lib test-dlmopen              libiamnotroot.so
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)                                                                                                                    test-dlmopen              rootfs/usr/local/lib/libiamnotroot.so
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs ld_library_path=$(CURDIR)/rootfs/usr/lib                test-dlmopen              /usr/local/lib/libiamnotroot.so
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/usr/bin                                                                                   test-dladdr               dladdr                                                | tee /dev/stderr | grep '^$(LIBRARY)$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-dladdr               dladdr                                                | tee /dev/stderr | grep '^$(LIBRARY)$$'
ifneq ($(OS),NetBSD)
	env LD_PRELOAD=$(LIBRARY):libiamnotroot.so PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)                                                                                                   test-dladdr  unused           | tee /dev/stderr | grep '^$(CURDIR)/libiamnotroot.so$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY):libiamnotroot.so LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib:$(CURDIR)/rootfs/usr/local/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs         test-dladdr  unused           | tee /dev/stderr | grep '^/usr/local/lib/libiamnotroot.so$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY):$(CURDIR)/rootfs/usr/local/lib/libiamnotroot.so LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs         test-dladdr  unused           | tee /dev/stderr | grep '^/usr/local/lib/libiamnotroot.so$$'
endif
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/usr/bin                                                                                   test-dladdr1              dladdr1                                               | tee /dev/stderr | grep '^$(LIBRARY):$(LIBRARY)$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-dladdr1              dladdr1                                               | tee /dev/stderr | grep '^$(LIBRARY):$(LIBRARY)$$'
	env LD_PRELOAD=$(LIBRARY):libiamnotroot.so PATH=$(CURDIR)/rootfs/usr/bin LD_LIBRARY_PATH=$(CURDIR)                                                                                                   test-dladdr1 unused           | tee /dev/stderr | grep '^$(CURDIR)/libiamnotroot.so:$(CURDIR)/libiamnotroot.so$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY):libiamnotroot.so LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib:$(CURDIR)/rootfs/usr/local/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs         test-dladdr1 unused           | tee /dev/stderr | grep '^/usr/local/lib/libiamnotroot.so:$(CURDIR)/rootfs/usr/local/lib/libiamnotroot.so$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY):$(CURDIR)/rootfs/usr/local/lib/libiamnotroot.so LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs         test-dladdr1 unused           | tee /dev/stderr | grep '^/usr/local/lib/libiamnotroot.so:$(CURDIR)/rootfs/usr/local/lib/libiamnotroot.so$$'
endif
	! env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-dl_iterate_phdr                                                            | grep -v '$(CURDIR)/rootfs/usr/bin/test-dl_iterate_phdr' | grep '$(CURDIR)/rootfs'
	cd rootfs/ && ! env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs     test-dl_iterate_phdr                                                            | grep -v '$(CURDIR)/rootfs/usr/bin/test-dl_iterate_phdr' | grep '$(CURDIR)/rootfs'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-getgid                                                                     | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_GID=1000                                                                      test-getgid                                                                     | tee /dev/stderr | grep '^1000$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-getegid                                                                    | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_EGID=1000                                                                     test-getegid                                                                    | tee /dev/stderr | grep '^1000$$'
ifneq ($(OS),NetBSD)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-getresgid                                                                  | tee /dev/stderr | grep '^0:0:0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_SGID=1000                                                                     test-getresgid                                                                  | tee /dev/stderr | grep '^0:0:1000$$'
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setgid     0                                                               | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setegid    0                                                               | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setregid   0  0                                                            | tee /dev/stderr | grep '^0:0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setregid   0 -1                                                            | tee /dev/stderr | grep '^0:$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setregid  -1  0                                                            | tee /dev/stderr | grep '^:0$$'
ifneq ($(OS),NetBSD)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setresgid  0  0  0                                                         | tee /dev/stderr | grep '^0:0:0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setresgid  0  0 -1                                                         | tee /dev/stderr | grep '^0:0:$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setresgid  0 -1  0                                                         | tee /dev/stderr | grep '^0::0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setresgid -1  0  0                                                         | tee /dev/stderr | grep '^:0:0$$'
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-getuid                                                                     | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_UID=1000                                                                      test-getuid                                                                     | tee /dev/stderr | grep '^1000$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-geteuid                                                                    | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_EUID=1000                                                                     test-geteuid                                                                    | tee /dev/stderr | grep '^1000$$'
ifneq ($(OS),NetBSD)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-getresgid                                                                  | tee /dev/stderr | grep '^0:0:0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin IAMROOT_SGID=1000                                                                     test-getresgid                                                                  | tee /dev/stderr | grep '^0:0:1000$$'
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setuid     0                                                               | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-seteuid    0                                                               | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setreuid   0  0                                                            | tee /dev/stderr | grep '^0:0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setreuid   0 -1                                                            | tee /dev/stderr | grep '^0:$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setreuid  -1 -1                                                            | tee /dev/stderr | grep '^:$$'
ifneq ($(OS),NetBSD)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setresuid  0  0  0                                                         | tee /dev/stderr | grep '^0:0:0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setresuid  0  0 -1                                                         | tee /dev/stderr | grep '^0:0:$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setresuid  0 -1  0                                                         | tee /dev/stderr | grep '^0::0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setresuid -1  0  0                                                         | tee /dev/stderr | grep '^:0:0$$'
endif
ifeq ($(OS),GNU/Linux)
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setfsuid   0                                                               | tee /dev/stderr | grep '^0$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-setfsgid   0                                                               | tee /dev/stderr | grep '^0$$'
endif
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-posix_spawn -- /bin/sh -c 'echo "$$@"' sh one two three                    | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-posix_spawn -- /bin/sh -c 'echo "$$@"' sh one two three                    | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-execl                                                                      | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-execl                                                                      | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-execle                                                                     | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-execle                                                                     | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-execlp                                                                     | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-execlp                                                                     | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-execv                                                                      | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-execv                                                                      | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-execve                                                                     | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-execve                                                                     | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-execveat                                                                   | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-execveat                                                                   | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-execvp                                                                     | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-execvp                                                                     | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-execvpe                                                                    | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-execvpe                                                                    | tee /dev/stderr | grep '^one two three$$'
	env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin:/bin                                                                                       test-fexecve                                                                    | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && PATH=$(CURDIR)/rootfs/usr/bin env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib IAMROOT_ROOT=$(CURDIR)/rootfs PWD=/ test-fexecve                                                                    | tee /dev/stderr | grep '^one two three$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getpwnam   root                                                            | tee /dev/stderr | grep -q '^root:.*:0:0:.*:.*:.*$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getpwnam_r root                                                            | tee /dev/stderr | grep -q '^root:.*:0:0:.*:.*:.*$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getpwuid   0                                                               | tee /dev/stderr | grep -q '^root:.*:0:0:.*:.*:.*$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getpwuid_r 0                                                               | tee /dev/stderr | grep -q '^root:.*:0:0:.*:.*:.*$$'
ifeq ($(OS),GNU/Linux)
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fgetpwent                                     <$(CURDIR)/rootfs/etc/passwd
endif
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getpwent
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-setpwent
ifeq ($(OS),GNU/Linux)
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-putpwent iamroot x 1000 1000 "iamroot user" /home/iamroot /usr/bin/ish     | tee /dev/stderr | grep -q '^iamroot:x:1000:1000:iamroot user:/home/iamroot:/usr/bin/ish$$'
endif
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-endpwent
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgrnam   root                                                            | tee /dev/stderr | grep -q '^root:.*:0:.*$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgrnam_r root                                                            | tee /dev/stderr | grep -q '^root:.*:0:.*$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgrgid   0                                                               | tee /dev/stderr | grep -q '^root:.*:0:.*$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgrgid_r 0                                                               | tee /dev/stderr | grep -q '^root:.*:0:.*$$'
ifeq ($(OS),GNU/Linux)
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fgetgrent                                      <$(CURDIR)/rootfs/etc/group
endif
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgrent
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-setgrent
ifeq ($(OS),GNU/Linux)
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-putgrent iamroot x 1000                                                    | tee /dev/stderr | grep -q '^iamroot:x:1000:$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-putgrent iamroot x 1000 iamroot                                            | tee /dev/stderr | grep -q '^iamroot:x:1000:iamroot$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-putgrent iamroot x 1000 iamroot wheel                                      | tee /dev/stderr | grep -q '^iamroot:x:1000:iamroot,wheel$$'
endif
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-endgrent
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgroups                                                                  | tee /dev/stderr | grep -q '^1 0$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs   IAMROOT_GROUPS="0 100 1000"                  test-getgroups                                                                  | tee /dev/stderr | grep -q '^3 0 100 1000$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs   IAMROOT_GROUPS="0 100 1000"                  test-getgroups 3                                                                | tee /dev/stderr | grep -q '^3 0 100 1000$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-initgroups   "$$USER" "$$GROUPS"
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgrouplist "$$USER" "$$GROUPS"
ifneq (,$(findstring $(OS),GNU/Linux Linux))
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgrouplist "$$USER" "$$GROUPS" 0
else
	cd rootfs/ && ! env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs     test-getgrouplist "$$USER" "$$GROUPS" 0
endif
ifeq ($(OS),NetBSD)
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getgroupmembership "$$USER" "$$GROUPS"
	cd rootfs/ && ! env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib  PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs    test-getgroupmembership "$$USER" "$$GROUPS" 0
endif
	cd rootfs/ && ! env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs IAMROOT_GROUPS="0 100 1000"                  test-getgroups 2
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs   IAMROOT_GROUPS="0 100 1000"                  test-setgroups                                                                  | tee /dev/stderr | grep -q '^$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-setgroups 0 100 1000                                                       | tee /dev/stderr | grep -q '^0 100 1000$$'
ifeq ($(OS),GNU/Linux)
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getspnam   root                                                            | tee /dev/stderr | grep -q 'root:\*:14871::::::$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-getspnam_r root                                                            | tee /dev/stderr | grep -q '^root:\*:14871::::::$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-fgetspent                                     <$(CURDIR)/rootfs/etc/shadow
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-setspent
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-putspent root '*' '14871' -1 -1 -1 -1 -1 -1                                | tee /dev/stderr | grep -q '^root:\*:14871::::::$$'
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-endspent
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-lckpwdf
	cd rootfs/ && env LD_PRELOAD=$(LIBRARY) LD_LIBRARY_PATH=$(CURDIR)/rootfs/usr/lib PATH=$(CURDIR)/rootfs/usr/bin IAMROOT_ROOT=$(CURDIR)/rootfs       test-ulckpwdf
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fsetxattr            rootfs/etc/os-release user.one   0 0
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fsetxattr            rootfs/etc/os-release user.one   1 0
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fsetxattr            rootfs/etc/os-release user.two   1 1
	! env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fsetxattr            rootfs/etc/os-release user.two   2 1
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fsetxattr            rootfs/etc/os-release user.two   2 0
	! env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fsetxattr            rootfs/etc/os-release user.three 1 2
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fsetxattr            rootfs/etc/os-release user.three 1 1
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fsetxattr            rootfs/etc/os-release user.three 3 2
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-flistxattr           rootfs/etc/os-release                                 | tee /dev/stderr | grep -q '^user.three:user.two:user.one:$$'
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fgetxattr            rootfs/etc/os-release user.one                        | tee /dev/stderr | grep -q '^1$$'
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fgetxattr            rootfs/etc/os-release user.two                        | tee /dev/stderr | grep -q '^2$$'
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fgetxattr            rootfs/etc/os-release user.three                      | tee /dev/stderr | grep -q '^3$$'
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fremovexattr         rootfs/etc/os-release user.three
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fremovexattr         rootfs/etc/os-release user.two
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-fremovexattr         rootfs/etc/os-release user.one
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-setxattr             rootfs/etc/os-release user.one   0 0
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-setxattr             rootfs/etc/os-release user.one   1 0
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-setxattr             rootfs/etc/os-release user.two   1 1
	! env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-setxattr             rootfs/etc/os-release user.two   2 1
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-setxattr             rootfs/etc/os-release user.two   2 0
	! env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-setxattr             rootfs/etc/os-release user.three 1 2
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-setxattr             rootfs/etc/os-release user.three 1 1
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-setxattr             rootfs/etc/os-release user.three 3 2
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-listxattr            rootfs/etc/os-release                                 | tee /dev/stderr | grep -q '^user.three:user.two:user.one:$$'
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-getxattr             rootfs/etc/os-release user.one                        | tee /dev/stderr | grep -q '^1$$'
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-getxattr             rootfs/etc/os-release user.two                        | tee /dev/stderr | grep -q '^2$$'
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-getxattr             rootfs/etc/os-release user.three                      | tee /dev/stderr | grep -q '^3$$'
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-removexattr          rootfs/etc/os-release user.three
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-removexattr          rootfs/etc/os-release user.two
	  env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-removexattr          rootfs/etc/os-release user.one
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lsetxattr            rootfs/etc/os-release user.one   0 0
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lsetxattr            rootfs/etc/os-release user.one   1 0
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lsetxattr            rootfs/etc/os-release user.two   1 1
	! env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lsetxattr            rootfs/etc/os-release user.two   2 1
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lsetxattr            rootfs/etc/os-release user.two   2 0
	! env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lsetxattr            rootfs/etc/os-release user.three 1 2
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lsetxattr            rootfs/etc/os-release user.three 1 1
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lsetxattr            rootfs/etc/os-release user.three 3 2
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-llistxattr           rootfs/etc/os-release                                 | tee /dev/stderr | grep -q '^user.three:user.two:user.one:$$'
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lgetxattr            rootfs/etc/os-release user.one                        | tee /dev/stderr | grep -q '^1$$'
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lgetxattr            rootfs/etc/os-release user.two                        | tee /dev/stderr | grep -q '^2$$'
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lgetxattr            rootfs/etc/os-release user.three                      | tee /dev/stderr | grep -q '^3$$'
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lremovexattr         rootfs/etc/os-release user.three
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lremovexattr         rootfs/etc/os-release user.two
	- env LD_PRELOAD=$(LIBRARY) PATH=$(CURDIR)/rootfs/usr/bin                                                                                          test-lremovexattr         rootfs/etc/os-release user.one
endif

.PHONY: test-libiamroot.so
test-libiamroot.so: | rootfs
	bash tests-libiamroot.so.bash

.PHONY: test-frontends
test-frontends: test-ld-iamroot.so test-ido test-ish

.PHONY: test-ld-iamroot.so
test-ld-iamroot.so: | rootfs
ifeq ($(OS),GNU/Linux)
	bash tests-ld-iamroot.so.bash
endif

.PHONY: test-ido
test-ido: | rootfs
ifeq ($(OS),GNU/Linux)
	bash tests-ido.bash
endif

.PHONY: test-ish
test-ish: | rootfs
ifeq ($(OS),GNU/Linux)
	bash tests-ish.bash
endif

rootfs: DESTDIR = $(CURDIR)/rootfs.tmp
rootfs:
	rm -Rf $(DESTDIR)
	install -d -m755 $(DESTDIR)
	install -m644 Makefile $(DESTDIR)/
	install -d -m 755 $(DESTDIR)/etc
	install -m644 passwd $(DESTDIR)/etc/
	install -m644 group  $(DESTDIR)/etc/
	install -m600 shadow $(DESTDIR)/etc/
	install -d -m755 $(DESTDIR)/usr/bin
	ln -sf usr/bin $(DESTDIR)/bin
ifneq ($(OS),NetBSD)
	install -d -m755 $(DESTDIR)/usr/local
	ln -sf ../../bin $(DESTDIR)/usr/local/bin
else
	install -d -m755 $(DESTDIR)/usr/pkg
	ln -sf ../../bin $(DESTDIR)/usr/pkg/bin
	ln -sf pkg $(DESTDIR)/usr/local
endif
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
ifeq ($(OS),OpenBSD)
	install -d -m755 $(DESTDIR)/usr/libexec $(DESTDIR)/usr/lib $(DESTDIR)/usr/local/lib
ifeq ($(KVER),7.2)
	install -m444 /usr/lib/libc.so.96.2 /usr/lib/libpthread.so.26.2 $(DESTDIR)/usr/lib
endif
ifeq ($(KVER),7.3)
	install -m444 /usr/lib/libc.so.97.0 /usr/lib/libpthread.so.27.0 $(DESTDIR)/usr/lib
endif
ifeq ($(KVER),7.4)
	install -m444 /usr/lib/libc.so.97.1 /usr/lib/libpthread.so.27.1 $(DESTDIR)/usr/lib
endif
ifeq ($(KVER),7.5)
	install -m444 /usr/lib/libc.so.99.0 /usr/lib/libpthread.so.27.1 $(DESTDIR)/usr/lib
endif
	install -m444 /usr/libexec/ld.so $(DESTDIR)/usr/libexec
	install -m444 libiamnotroot.so $(DESTDIR)/usr/local/lib
endif
ifeq ($(OS),NetBSD)
	install -d -m755 $(DESTDIR)/libexec $(DESTDIR)/lib $(DESTDIR)/usr/libexec $(DESTDIR)/usr/pkg/lib
	install -m444 /lib/libc.so.12.213 $(DESTDIR)/lib
	ln -sf libc.so.12.213 $(DESTDIR)/lib/libc.so.12
	ln -sf libc.so.12.213 $(DESTDIR)/lib/libc.so
	install -m555 /libexec/ld.elf_so $(DESTDIR)/libexec
	ln -sf /libexec/ld.elf_so $(DESTDIR)/usr/libexec/ld.elf_so
	install -m444 libiamnotroot.so $(DESTDIR)/usr/pkg/lib
endif
ifeq ($(OS),GNU/Linux)
ifeq ($(shell test -d /lib/$(ARCH)-linux-gnu/ && echo 1),1)
	install -D -m755 /lib/$(ARCH)-linux-gnu/libc.so.6 /lib/$(ARCH)-linux-gnu/libdl.so.2 /lib/$(ARCH)-linux-gnu/libpthread.so.0 --target-directory=$(DESTDIR)/usr/lib64
	ln -sf lib64 $(DESTDIR)/usr/lib
	ln -sf . $(DESTDIR)/usr/lib/$(ARCH)-linux-gnu
else
	install -D -m755 /usr/lib64/libc.so.6 /usr/lib64/libdl.so.2 /usr/lib64/libpthread.so.0 --target-directory=$(DESTDIR)/usr/lib64
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
	install -D -m755 libiamnotroot.so --target-directory=$(DESTDIR)/usr/local/lib
endif
ifeq ($(OS),Linux)
	install -D -m755 /lib/ld-musl-x86_64.so.1 --target-directory=$(DESTDIR)/lib
	install -D -m755 libiamnotroot.so --target-directory=$(DESTDIR)/usr/local/lib
endif
ifeq ($(OS),GNU/Linux)
	echo "/usr/local/lib" >$(DESTDIR)/etc/ld.so.conf
	ldconfig -v -r $(DESTDIR)
endif
	install -d -m755 $(DESTDIR)/usr/lib/
	install -m644 os-release $(DESTDIR)/usr/lib/
	ln -sf ../usr/lib/os-release $(DESTDIR)/etc/os-release
	install -d -m755 $(DESTDIR)/proc
	install -d -m755 $(DESTDIR)/sys
	install -d -m755 $(DESTDIR)/dev
	install -d -m777 $(DESTDIR)/tmp
	install -d -m755 $(DESTDIR)/run
	install -d -m755 $(DESTDIR)/var
	install -d -m755 $(DESTDIR)/var/log
ifeq ($(OS),FreeBSD)
	install -m 644 os-release $(DESTDIR)/usr/lib/
	ln -sf ../usr/lib/os-release $(DESTDIR)/etc/os-release
	install -d -m755 $(DESTDIR)/var/run
endif
ifeq ($(OS),GNU/Linux)
	ln -sf /run $(DESTDIR)/var/run
endif
ifneq ($(OS),Linux)
	install -d -m755 $(DESTDIR)/usr/lib/nls/msg/$$LANG/
	gencat $(DESTDIR)/usr/lib/nls/msg/$$LANG/test.cat test.msg
endif
	bash install-bin bash $(DESTDIR)
	bash install-bin env $(DESTDIR)
	ln -sf bash $(DESTDIR)/usr/bin/sh
ifeq ($(XATTR),xattr)
	install -d -m755 $(DESTDIR)/linux/proc
	getfattr -n user.iamroot.path-resolution -v "/proc\0" $(DESTDIR)/linux/proc
endif
	mv $(DESTDIR) $@

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

# vim: cc=114
