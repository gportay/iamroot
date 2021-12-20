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
test: export LD_LIBRARY_PATH := $(CURDIR):$(LD_LIBRARY_PATH)
test: export IAMROOT_USE_HOST_INTERP = 1
test: ALPINE_MINIROOTFS ?= .
test:
	rm -Rf proc sys tmp dev* file* symlink[0-9] link[0-9]*
	touch file
	ln -s file symlink0
	test-access Makefile 0
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-access /Makefile 0
	IAMROOT_USE_HOST_INTERP= test-chroot $(ALPINE_MINIROOTFS) whoami
	test-chmod file 01644
	test-fchmodat . file 02644
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-fchmodat - /file 04644
	test-chown file 0 0
	test-faccessat . Makefile 0 0
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-faccessat - /Makefile 0 0
	test-fchown file 0 0
	test-fchownat - file 0 0
	test-stat file
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-stat /file
	test-lstat symlink0
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-lstat /symlink0
	test-fstat file
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-fstat file
	test-fstatat - file 0 0
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-fstatat - /file 0 0
	test-lchown symlink0 0 0
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-fchownat - /file 0 0
	test-get_current_dir_name | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-get_current_dir_name | tee /dev/stderr | grep -q '/'
	test-getcwd | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-getcwd | tee /dev/stderr | grep -q '/'
	test-getwd | tee /dev/stderr | grep -q '$(CURDIR)'
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-getwd | tee /dev/stderr | grep -q '/'
	test-link file link1
	test-linkat - file - link2 0
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-linkat - /file - /link3 0
	test-mkdir proc 0555
	test-mkdirat . sys 0555
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-mkdirat . /tmp 0755
	test-mknod dev 0644 0 0
	test-mknodat - dev1 0666 0 1
	test-mknodat . dev2 0666 0 2
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-mknodat - /dev3 0666 0 3
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-mknodat . /dev4 0666 0 4
	test-mount -t procfs none /proc
	test-path_resolution / | tee /dev/stderr | grep -q "^/\$$"
	test-path_resolution /proc/ | tee /dev/stderr | grep -q "^/proc/\$$"
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-path_resolution /sys/ | tee /dev/stderr | grep -q "^/sys/\$$"
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-path_resolution /dev/ | tee /dev/stderr | grep -q "^/dev/\$$"
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-path_resolution /run/ | tee /dev/stderr | grep -q "^/run/\$$"
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-path_resolution /etc/passwd | tee /dev/stderr | grep -q "^$(CURDIR)/etc/passwd\$$"
	IAMROOT_PATH_RESOLUTION_IGNORE="^/(proc|sys)/|$$IAMROOT_LIB" \
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-path_resolution "$$IAMROOT_LIB"  | tee /dev/stderr | grep -q "^$$IAMROOT_LIB$$"
	test-pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^/usr/local/lib:/usr/lib:/lib$$"
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^$(CURDIR)/usr/local/lib:$(CURDIR)/usr/lib:$(CURDIR)/lib$$"
	test-openat - file
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-openat . /file
	test-opendir proc
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-opendir /proc
	test-rename file file1
	test-renameat - file1 - file2
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-renameat - /file2 - /file3
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-renameat2 - /file3 - /file 0
	test-rmdir proc
	test-rmdir sys
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-rmdir /tmp
	test-sanitize ""                  | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	test-sanitize . ./ ././ ./. ././. | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	test-sanitize / // ///  //. //./. | tee /dev/stderr | grep -vc '^/$$'  | grep -q '^0$$'
	test-scandir .
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-scandirat - /
	test-statfs .
	test-statvfs .
	test-statx - file 0 0
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-statx - /file 0 0
	test-symlink file symlink1
	test-symlinkat file . symlink2
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-symlinkat file . /symlink3
	test-utimes file 0x3fffffff 0x3fffffff
	test-lutimes symlink0 0x3fffffff 0x3fffffff
	test-utimensat . file 0x3fffffff 0x3fffffff
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-utimensat . /file 0x3fffffff 0x3fffffff
	test-unlink file
	test-unlinkat . dev1 0
	PATH=/ IAMROOT_ROOT=$(CURDIR) /test-unlinkat - /dev2 0
	test-unshare whoami

.PHONY: clean
clean:
	rm -f libiamnotroot.so *.o test-*

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

test-%: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
