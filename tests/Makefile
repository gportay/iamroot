#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

override CFLAGS += -fPIC -Wall -Wextra -Werror
override CFLAGS += -D_GNU_SOURCE -Wno-error=deprecated-declarations

TESTS += access
TESTS += chmod
TESTS += chown
TESTS += chroot
TESTS += execl
TESTS += execle
TESTS += execlp
TESTS += execv
TESTS += execvp
TESTS += execvpe
TESTS += faccessat
TESTS += fchmodat
TESTS += fchown
TESTS += fchownat
TESTS += fexecve
TESTS += fstat
TESTS += fstatat
TESTS += get_current_dir_name
TESTS += getcwd
TESTS += getwd
TESTS += lchown
TESTS += link
TESTS += linkat
TESTS += lstat
TESTS += lutimes
TESTS += mkdir
TESTS += mkdirat
TESTS += mknod
TESTS += mknodat
TESTS += mount
TESTS += openat
TESTS += opendir
TESTS += path_resolution
TESTS += pathsetenv
TESTS += rename
TESTS += renameat
TESTS += renameat2
TESTS += rmdir
TESTS += sanitize
TESTS += scandir
TESTS += scandirat
TESTS += stat
TESTS += statfs
TESTS += statvfs
TESTS += statx
TESTS += symlink
TESTS += symlinkat
TESTS += unlink
TESTS += unlinkat
TESTS += unshare
TESTS += utimensat
TESTS += utimes

.PHONY: all
all: libiamnotroot.so
all: $(TESTS)

libiamnotroot.so: iamnotroot.o
libiamnotroot.so: override LDLIBS += -ldl

path_resolution: libiamnotroot.so
path_resolution: override LDLIBS += libiamnotroot.so

pathsetenv: libiamnotroot.so
pathsetenv: override LDLIBS += libiamnotroot.so

.PHONY: test
test: PATH := $(CURDIR):$(PATH)
test: export LD_LIBRARY_PATH := $(CURDIR):$(LD_LIBRARY_PATH)
test: export IAMROOT_USE_HOST_INTERP = 1
test: ALPINE_MINIROOTFS ?= .
test:
	rm -Rf proc sys tmp dev* file* symlink[0-9] link[0-9]*
	touch file
	ln -s file symlink0
	access Makefile 0
	IAMROOT_ROOT=$(CURDIR) /access /Makefile 0
	IAMROOT_USE_HOST_INTERP= chroot $(ALPINE_MINIROOTFS) whoami
	chmod file 01644
	fchmodat . file 02644
	IAMROOT_ROOT=$(CURDIR) /fchmodat - /file 04644
	chown file 0 0
	faccessat . Makefile 0 0
	IAMROOT_ROOT=$(CURDIR) /faccessat - /Makefile 0 0
	fchown file 0 0
	fchownat - file 0 0
	stat file
	IAMROOT_ROOT=$(CURDIR) /stat /file
	lstat symlink0
	IAMROOT_ROOT=$(CURDIR) /lstat /symlink0
	fstat file
	IAMROOT_ROOT=$(CURDIR) /fstat file
	fstatat - file 0 0
	IAMROOT_ROOT=$(CURDIR) /fstatat - /file 0 0
	lchown symlink0 0 0
	IAMROOT_ROOT=$(CURDIR) /fchownat - /file 0 0
	get_current_dir_name | tee /dev/stderr | grep -q '$(CURDIR)'
	IAMROOT_ROOT=$(CURDIR) /get_current_dir_name | tee /dev/stderr | grep -q '/'
	getcwd | tee /dev/stderr | grep -q '$(CURDIR)'
	IAMROOT_ROOT=$(CURDIR) /getcwd | tee /dev/stderr | grep -q '/'
	getwd | tee /dev/stderr | grep -q '$(CURDIR)'
	IAMROOT_ROOT=$(CURDIR) /getwd | tee /dev/stderr | grep -q '/'
	link file link1
	linkat - file - link2 0
	IAMROOT_ROOT=$(CURDIR) /linkat - /file - /link3 0
	mkdir proc 0555
	mkdirat . sys 0555
	IAMROOT_ROOT=$(CURDIR) /mkdirat . /tmp 0755
	mknod dev 0644 0 0
	mknodat - dev1 0666 0 1
	mknodat . dev2 0666 0 2
	IAMROOT_ROOT=$(CURDIR) /mknodat - /dev3 0666 0 3
	IAMROOT_ROOT=$(CURDIR) /mknodat . /dev4 0666 0 4
	mount -t procfs none /proc
	path_resolution / | tee /dev/stderr | grep -q "^/\$$"
	path_resolution /proc/ | tee /dev/stderr | grep -q "^/proc/\$$"
	IAMROOT_ROOT=$(CURDIR) /path_resolution /sys/ | tee /dev/stderr | grep -q "^/sys/\$$"
	IAMROOT_ROOT=$(CURDIR) /path_resolution /dev/ | tee /dev/stderr | grep -q "^/dev/\$$"
	IAMROOT_ROOT=$(CURDIR) /path_resolution /run/ | tee /dev/stderr | grep -q "^/run/\$$"
	IAMROOT_ROOT=$(CURDIR) /path_resolution /etc/passwd | tee /dev/stderr | grep -q "^$(CURDIR)/etc/passwd\$$"
	IAMROOT_PATH_RESOLUTION_IGNORE="^/(proc|sys)/|$$IAMROOT_LIB" \
	IAMROOT_ROOT=$(CURDIR) /path_resolution "$$IAMROOT_LIB"  | tee /dev/stderr | grep -q "^$$IAMROOT_LIB$$"
	pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^/usr/local/lib:/usr/lib:/lib$$"
	IAMROOT_ROOT=$(CURDIR) /pathsetenv LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib  | tee /dev/stderr | grep -q "^$(CURDIR)/usr/local/lib:$(CURDIR)/usr/lib:$(CURDIR)/lib$$"
	openat - file
	IAMROOT_ROOT=$(CURDIR) /openat . /file
	opendir proc
	IAMROOT_ROOT=$(CURDIR) /opendir /proc
	rename file file1
	renameat - file1 - file2
	IAMROOT_ROOT=$(CURDIR) /renameat - /file2 - /file3
	IAMROOT_ROOT=$(CURDIR) /renameat2 - /file3 - /file 0
	rmdir proc
	rmdir sys
	IAMROOT_ROOT=$(CURDIR) /rmdir /tmp
	sanitize ""                  | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	sanitize . ./ ././ ./. ././. | tee /dev/stderr | grep -vc '^\.$$' | grep -q '^0$$'
	sanitize / // ///  //. //./. | tee /dev/stderr | grep -vc '^/$$'  | grep -q '^0$$'
	scandir .
	IAMROOT_ROOT=$(CURDIR) /scandirat - /
	statfs .
	statvfs .
	statx - file 0 0
	IAMROOT_ROOT=$(CURDIR) /statx - /file 0 0
	symlink file symlink1
	symlinkat file . symlink2
	IAMROOT_ROOT=$(CURDIR) /symlinkat file . /symlink3
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
	rm -f libiamnotroot.so *.o $(TESTS)

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
