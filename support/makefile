#
# Copyright 2021-2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

PREFIX ?= /usr/local
KVER ?= $(shell uname -r 2>/dev/null)
VMLINUX_KVER ?= $(shell vmlinux --version 2>/dev/null)

-include local.mk

MAKEFLAGS += --no-print-directory

.PHONY: all
all:

.PRECIOUS: libiamroot-%.so.1 libiamroot-%.so.2 libiamroot-%.so.3
libiamroot-%.so.1 libiamroot-%.so.2 libiamroot-%.so.3: output-%/libiamroot.so
	cp output-$*/libiamroot.so $@

.PRECIOUS: output-%/libiamroot.so
output-%/libiamroot.so: $(wildcard *.c) | output-%
	$(MAKE) -f $(CURDIR)/Makefile -C output-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: output-%
output-%:
	mkdir $@

.NOTPARALLEL: rootfs
.PHONY: rootfs
rootfs: i686-rootfs

.PHONY: i686-rootfs
i686-rootfs:

.PHONY: aarch64-rootfs
aarch64-rootfs:

.PHONY: arm-rootfs
arm-rootfs:

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf output-*/
	rm -f libiamroot*.so* *.ext4 *.cpio
	rm -Rf *-rootfs/

.PRECIOUS: libiamroot-linux-x86-64.so
libiamroot-linux-x86-64.so: libiamroot-linux-x86-64.so.2
	ln -sf $< $@

libiamroot-linux-x86-64.so.2:

.PRECIOUS: libiamroot-linux-x86.so
libiamroot-linux-x86.so: libiamroot-linux-x86.so.2
	ln -sf $< $@

all: libiamroot-linux.so.2

libiamroot-linux.so.2: export CC += -m32
# FIXES: undefined reference to `__stack_chk_fail_local'
libiamroot-linux.so.2: export CFLAGS += -fno-stack-protector
libiamroot-linux.so.2:

install: install-exec-linux

.PHONY: install-exec-linux
install-exec-linux:
	install -D -m 755 libiamroot-linux.so.2 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux.so.2
	ln -sf libiamroot-linux.so.2 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux.so

uninstall: uninstall-linux

.PHONY: uninstall-linux
uninstall-linux:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux.so.2
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux.so

ifneq ($(shell command -v pacstrap),)
.PHONY: arch-test
arch-test: | arch-rootfs/usr/bin/shebang.sh
arch-test: | arch-rootfs/usr/bin/shebang-arg.sh
arch-test: libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "chroot arch-rootfs shebang.sh one two three"
	bash iamroot-shell -c "chroot arch-rootfs shebang-arg.sh one two three"

arch-rootfs/usr/bin/%: support/% | arch-rootfs
	cp $< $@

.PHONY: arch-chroot
arch-chroot: | arch-rootfs
	bash iamroot-shell -c "chroot arch-rootfs"

rootfs: arch-rootfs

.PHONY: arch-rootfs
arch-rootfs: | arch-rootfs/etc/machine-id

arch-rootfs/etc/machine-id: | libiamroot-linux-x86-64.so.2
	mkdir -p arch-rootfs
	bash iamroot-shell -c "pacstrap -C support/x86_64-pacman.conf arch-rootfs"

i686-rootfs: i686-arch-rootfs

.PHONY: i686-arch-chroot
i686-arch-chroot: export QEMU_LD_PREFIX = $(CURDIR)/i686-arch-rootfs
i686-arch-chroot: export IAMROOT_LD_PRELOAD_LINUX_2 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
i686-arch-chroot: | i686-arch-rootfs
	bash iamroot-shell -c "chroot i686-arch-rootfs"

i686-rootfs: i686-arch-rootfs

.PHONY: i686-arch-rootfs
i686-arch-rootfs: export QEMU_LD_PREFIX = $(CURDIR)/i686-arch-rootfs
i686-arch-rootfs: | i686-arch-rootfs/etc/machine-id

i686-arch-rootfs/etc/machine-id: export QEMU_LD_PREFIX = $(CURDIR)/i686-arch-rootfs
i686-arch-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_2 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
i686-arch-rootfs/etc/machine-id: | libiamroot-linux.so.2 libiamroot-linux-x86-64.so.2
	mkdir -p i686-arch-rootfs
	bash iamroot-shell -c "pacstrap -C support/i686-pacman.conf i686-arch-rootfs"

qemu-system-x86_64-arch:

ifneq ($(VMLINUX_KVER),)
vmlinux-arch:
endif

arch.ext4:

arch-postrootfs: | libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i arch-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i arch-rootfs/etc/shadow
	mkdir -p arch-rootfs/var/lib/systemd/linger
	rm -f arch-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell --chroot $(CURDIR)/arch-rootfs -c "systemctl enable getty@tty0.service"

chroot-arch:

mount-arch:

umount-arch:
endif

ifneq ($(shell command -v dnf),)
fedora-33-chroot:
fedora-34-chroot:
fedora-35-chroot:
fedora-%-chroot: export IAMROOT_PATH = /usr/bin:/usr/sbin:/bin:/sbin
fedora-%-chroot: export IAMROOT_LD_LIBRARY_PATH = /usr/lib64:/lib64
fedora-%-chroot: | fedora-%-rootfs
	bash iamroot-shell -c "chroot fedora-$*-rootfs"

rootfs: fedora-rootfs

.NOTPARALLEL: fedora-rootfs
.PHONY: fedora-rootfs
fedora-rootfs: fedora-33-rootfs fedora-34-rootfs fedora-35-rootfs

fedora-33-rootfs: | fedora-33-rootfs/etc/machine-id
fedora-34-rootfs: | fedora-34-rootfs/etc/machine-id
fedora-35-rootfs: | fedora-35-rootfs/etc/machine-id

fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH = /usr/bin:/usr/sbin:/bin:/sbin
fedora-%-rootfs/etc/machine-id: export IAMROOT_LD_LIBRARY_PATH = /usr/lib64:/lib64:/usr/lib64/ldb/modules/ldb:/usr/lib64/ldb
fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|run/.+)/|$(CURDIR)/fedora-$*-rootfs/var/log/dnf.rpm.log
fedora-%-rootfs/etc/machine-id: | libiamroot-linux-x86-64.so.2
	install -D -m644 support/fedora.repo fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --releasever $* --assumeyes --installroot $(CURDIR)/fedora-$*-rootfs group install minimal-environment"
	rm -f fedora-$*-rootfs/etc/distro.repos.d/fedora.repo

qemu-system-x86_64-fedora-33:
qemu-system-x86_64-fedora-33: override CMDLINE += rw
qemu-system-x86_64-fedora-34:
qemu-system-x86_64-fedora-34: override CMDLINE += rw
qemu-system-x86_64-fedora-35:
qemu-system-x86_64-fedora-35: override CMDLINE += rw

ifneq ($(VMLINUX_KVER),)
vmlinux-fedora-33:
vmlinux-fedora-34:
vmlinux-fedora-35:
endif

fedora-33.ext4 fedora-34.ext4 fedora-35.ext4:

fedora-33-postrootfs:
fedora-34-postrootfs:
fedora-35-postrootfs:
fedora-35-postrootfs: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /usr/lib64/libc.so.6:/usr/lib64/libdl.so.2:/usr/lib64/libpthread.so.0:/usr/lib64/librt.so.1
fedora-%-postrootfs: | libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i fedora-$*-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i fedora-$*-rootfs/etc/shadow
	touch fedora-$*-rootfs/etc/systemd/zram-generator.conf
	mkdir -p fedora-$*-rootfs/var/lib/systemd/linger
	rm -f fedora-$*-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell --chroot $(CURDIR)/fedora-$*-rootfs -c "systemctl enable getty@tty0.service"
	rm -f fedora-$*-rootfs/etc/systemd/system/multi-user.target.wants/sshd.service
	bash iamroot-shell --chroot $(CURDIR)/fedora-$*-rootfs -c "systemctl disable sshd.service"

chroot-fedora-33 chroot-fedora-34 chroot-fedora-35:

mount-fedora-33 mount-fedora-34 mount-fedora-35:

umount-fedora-33 umount-fedora-34 umount-fedora-35:
endif

ifneq ($(shell command -v clang),)
all: libiamroot-linux-x86-64-clang.so

.PRECIOUS: libiamroot-linux-x86-64-clang.so
libiamroot-linux-x86-64-clang.so: libiamroot-linux-x86-64-clang.so.2
	ln -sf $< $@

libiamroot-linux-x86-64-clang.so.2:
libiamroot-linux-x86-64-clang.so.2: export CC = clang
endif

ifneq ($(shell command -v musl-gcc),)
all ci: libiamroot-musl-x86_64.so

.PRECIOUS: libiamroot-musl-x86_64.so
libiamroot-musl-x86_64.so: libiamroot-musl-x86_64.so.1
	ln -sf $< $@

libiamroot-musl-x86_64.so.1:
libiamroot-musl-x86_64.so.1: export CC = musl-gcc

install: install-exec-musl-x86_64

.PHONY: install-exec-musl-x86_64
install-exec-musl-x86_64:
	install -D -m 755 libiamroot-musl-x86_64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so.1
	ln -sf libiamroot-musl-x86_64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so

uninstall: uninstall-musl-x86_64

.PHONY: uninstall-musl-x86_64
uninstall-musl-x86_64:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so.1
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so

.PHONY: alpine-test
alpine-test: | alpine-minirootfs/usr/bin/shebang.sh
alpine-test: | alpine-minirootfs/usr/bin/shebang-arg.sh
alpine-test: | alpine-minirootfs/usr/bin/shebang-busybox.sh
alpine-test: libiamroot-musl-x86_64.so.1 libiamroot-linux-x86-64.so.2 | alpine-minirootfs
	bash iamroot-shell -c "chroot alpine-minirootfs pwd" | tee /dev/stderr | grep -q "^/\$$"
	bash iamroot-shell -c "chroot alpine-minirootfs cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs chroot . cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell -c "chroot alpine-minirootfs /bin/busybox"
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs shebang.sh one two three"
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs shebang-arg.sh one two three"
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs shebang-busybox.sh one two three"
	bash iamroot-shell -c "chroot alpine-minirootfs /lib/ld-musl-x86_64.so.1 --preload "$$PWD/libiamroot-musl-x86_64.so.1" bin/busybox"

rootfs: alpine-rootfs

.PHONY: mini-chroot
mini-chroot: libiamroot-musl-x86_64.so.1 libiamroot-linux-x86-64.so.2 | alpine-minirootfs
	bash iamroot-shell -c "chroot alpine-minirootfs /bin/sh"

alpine-minirootfs/usr/bin/%: support/% | alpine-minirootfs
	cp $< $@

.PHONY: alpine-minirootfs
alpine-minirootfs: | alpine-minirootfs/bin/busybox

alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.13.0-x86_64.tar.gz
	mkdir -p alpine-minirootfs
	tar xf alpine-minirootfs-3.13.0-x86_64.tar.gz -C alpine-minirootfs

alpine-minirootfs-3.13.0-x86_64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v3.13/releases/x86_64/alpine-minirootfs-3.13.0-x86_64.tar.gz

ifneq ($(shell command -v alpine-make-rootfs),)
alpine-3.14-chroot alpine-edge-chroot:
alpine-%-chroot: | alpine-%-rootfs
	bash iamroot-shell -c "chroot alpine-$*-rootfs /bin/ash"

.NOTPARALLEL: alpine-rootfs
.PHONY: alpine-rootfs
alpine-rootfs: alpine-3.14-rootfs alpine-edge-rootfs

alpine-3.14-rootfs: | alpine-3.14-rootfs/bin/busybox
alpine-edge-rootfs: | alpine-edge-rootfs/bin/busybox

alpine-%-rootfs/bin/busybox: | libiamroot-musl-x86_64.so.1 libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "alpine-make-rootfs alpine-$*-rootfs --keys-dir /usr/share/apk/keys/x86_64 --mirror-uri http://nl.alpinelinux.org/alpine --branch $*"

qemu-system-x86_64-alpine-3.14 qemu-system-x86_64-alpine-edge:

ifneq ($(VMLINUX_KVER),)
vmlinux-alpine-3.14 vmlinux-alpine-edge:
endif

alpine-3.14.ext4 alpine-edge.ext4:

alpine-%-postrootfs:
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i alpine-$*-rootfs/etc/passwd
	sed -e '/^UNKNOWN$$:/d' \
	    -e '1iUNKNOWN' \
	    -i alpine-$*-rootfs/etc/securetty
	sed -e '/^tty1:/itty0::respawn:/sbin/getty 38400 tty0' \
	    -e '/^tty[1-9]:/s,^,#,' \
	    -e '/^#ttyS0:/s,^#,,g' \
	    -i alpine-$*-rootfs/etc/inittab
	chmod +r alpine-$*-rootfs/bin/bbsuid

chroot-alpine-%: PATH = /usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin
chroot-alpine-%: SHELL = /bin/sh
chroot-alpine-3.14 chroot-alpine-edge:

mount-alpine-3.14 mount-alpine-edge:

umount-alpine-3.14 umount-alpine-edge:
endif
endif

ifneq ($(shell command -v aarch64-linux-gnu-gcc),)
all: libiamroot-linux-aarch64.so

.PRECIOUS: libiamroot-musl-aarch64.so
libiamroot-linux-aarch64.so: libiamroot-linux-aarch64.so.1
	ln -sf $< $@

libiamroot-linux-aarch64.so.1:
libiamroot-linux-aarch64.so.1: export CC = aarch64-linux-gnu-gcc

install: install-exec-linux-aarch64

.PHONY: install-exec-linux-aarch64
install-exec-linux-aarch64:
	install -D -m 755 libiamroot-linux-aarch64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-aarch64.so.1
	ln -sf libiamroot-linux-aarch64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-aarch64.so

uninstall: uninstall-exec-linux-aarch64

.PHONY: uninstall-exec-linux-aarch64
uninstall-exec-linux-aarch64:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-aarch64.so.1
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-aarch64.so

ifneq ($(shell command -v armv7l-unknown-linux-gnueabihf-gcc),)
all: libiamroot-linux-armhf.so

.PRECIOUS: libiamroot-linux-armhf.so
libiamroot-linux-armhf.so: libiamroot-linux-armhf.so.3
	ln -sf $< $@

libiamroot-linux-armhf.so.3:
libiamroot-linux-armhf.so.3: export CC = armv7l-unknown-linux-gnueabihf-gcc

install: install-exec-linux-armhf

.PHONY: install-exec-linux-armhf
install-exec-linux-armhf:
	install -D -m 755 libiamroot-linux-armhf.so.3 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-armhf.so.3
	ln -sf libiamroot-linux-armhf.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-armhf.so

uninstall: uninstall-exec-linux-armhf

.PHONY: uninstall-exec-linux-armhf
uninstall-exec-linux-armhf:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-armhf.so.3
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-armhf.so
endif

ifneq ($(shell command -v pacstrap),)
.PHONY: aarch64-arch-chroot
aarch64-arch-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-arch-rootfs
aarch64-arch-chroot: export IAMROOT_LD_PRELOAD_LINUX_AARCH64_1 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
aarch64-arch-chroot: | aarch64-arch-rootfs
	bash iamroot-shell -c "chroot aarch64-arch-rootfs"

aarch64-rootfs: aarch64-arch-rootfs

.PHONY: aarch64-arch-rootfs
aarch64-arch-rootfs: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-arch-rootfs
aarch64-arch-rootfs: | aarch64-arch-rootfs/etc/machine-id

aarch64-arch-rootfs/etc/machine-id: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-arch-rootfs
aarch64-arch-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_AARCH64_1 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
aarch64-arch-rootfs/etc/machine-id: | libiamroot-linux-aarch64.so.1 libiamroot-linux-x86-64.so.2
	mkdir -p aarch64-arch-rootfs
	bash iamroot-shell -c "pacstrap -C support/aarch64-pacman.conf aarch64-arch-rootfs"

ifneq ($(shell command -v armv7l-unknown-linux-gnueabihf-gcc),)
.PHONY: armv7h-arch-chroot
armv7h-arch-chroot: export QEMU_LD_PREFIX = $(CURDIR)/armv7h-arch-rootfs
armv7h-arch-chroot: export IAMROOT_LIB_LINUX_ARMHF_3 = $(CURDIR)/libiamroot-linux-armhf.so.3
armv7h-arch-chroot: export IAMROOT_LD_PRELOAD_LINUX_ARMHF_3 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
armv7h-arch-chroot: | armv7h-arch-rootfs
	bash iamroot-shell -c "chroot armv7h-arch-rootfs"

arm-rootfs: armv7h-arch-rootfs

.PHONY: armv7h-arch-rootfs
armv7h-arch-rootfs: export QEMU_LD_PREFIX = $(CURDIR)/armv7h-arch-rootfs
armv7h-arch-rootfs: | armv7h-arch-rootfs/etc/machine-id

armv7h-arch-rootfs/etc/machine-id: export QEMU_LD_PREFIX = $(CURDIR)/armv7h-arch-rootfs
armv7h-arch-rootfs/etc/machine-id: export IAMROOT_LIB_LINUX_ARMHF_3 = $(CURDIR)/libiamroot-linux-armhf.so.3
armv7h-arch-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_ARMHF_3 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
armv7h-arch-rootfs/etc/machine-id: | libiamroot-linux-armhf.so.3 libiamroot-linux-x86-64.so.2
	mkdir -p armv7h-arch-rootfs
	bash iamroot-shell -c "pacstrap -C support/armv7h-pacman.conf armv7h-arch-rootfs"
endif
endif

ifneq ($(shell command -v dnf),)
aarch64-fedora-33-chroot:
aarch64-fedora-34-chroot:
aarch64-fedora-35-chroot:
aarch64-fedora-%-chroot: export IAMROOT_PATH = /usr/bin:/usr/sbin:/bin:/sbin
aarch64-fedora-%-chroot: export IAMROOT_LD_LIBRARY_PATH = /usr/lib64:/lib64
aarch64-fedora-%-chroot: | aarch64-fedora-%-rootfs
	bash iamroot-shell -c "chroot aarch64-fedora-$*-rootfs"

aarch64-rootfs: aarch64-fedora-rootfs

.PHONY: aarch64-fedora-rootfs
aarch64-fedora-rootfs: aarch64-fedora-33-rootfs aarch64-fedora-34-rootfs aarch64-fedora-35-rootfs

aarch64-fedora-33-rootfs: | aarch64-fedora-33-rootfs/etc/machine-id
aarch64-fedora-34-rootfs: | aarch64-fedora-34-rootfs/etc/machine-id
aarch64-fedora-35-rootfs: | aarch64-fedora-35-rootfs/etc/machine-id

aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH = /usr/bin:/usr/sbin:/bin:/sbin
aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_LD_LIBRARY_PATH = /usr/lib64:/lib64:/usr/lib64/ldb/modules/ldb:/usr/lib64/ldb
aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|run/.+)/|$(CURDIR)/aarch64-fedora-$*-rootfs/var/log/dnf.rpm.log
aarch64-fedora-%-rootfs/etc/machine-id: | libiamroot-linux-aarch64.so.1 libiamroot-linux-x86-64.so.2
	install -D -m644 support/fedora.repo aarch64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --forcearch aarch64 --releasever $* --assumeyes --installroot $(CURDIR)/aarch64-fedora-$*-rootfs group install minimal-environment"
	rm -f aarch64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
endif
endif

ifneq ($(shell command -v aarch64-linux-musl-gcc),)
all: libiamroot-musl-aarch64.so

.PRECIOUS: libiamroot-musl-aarch64.so
libiamroot-musl-aarch64.so: libiamroot-musl-aarch64.so.1
	ln -sf $< $@

libiamroot-musl-aarch64.so.1:
libiamroot-musl-aarch64.so.1: export CC = aarch64-linux-musl-gcc

install: install-exec-musl-aarch64

.PHONY: install-exec-musl-aarch64
install-exec-musl-aarch64:
	install -D -m 755 libiamroot-musl-aarch64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-aarch64.so.1
	ln -sf libiamroot-musl-aarch64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-aarch64.so

uninstall: uninstall-exec-musl-aarch64

.PHONY: uninstall-exec-musl-aarch64
uninstall-exec-musl-aarch64:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-aarch64.so.1
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-aarch64.so

aarch64-rootfs: aarch64-alpine-rootfs

.PHONY: aarch64-mini-chroot
aarch64-mini-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-mini-rootfs
aarch64-mini-chroot: | libiamroot-musl-aarch64.so.1 libiamroot-linux-x86-64.so.2 aarch64-alpine-minirootfs
	bash iamroot-shell -c "chroot aarch64-alpine-minirootfs /bin/sh"

.PHONY: aarch64-alpine-minirootfs
aarch64-alpine-minirootfs: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-alpine-minirootfs
aarch64-alpine-minirootfs: | aarch64-alpine-minirootfs/bin/busybox

aarch64-alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.13.0-aarch64.tar.gz
	mkdir -p aarch64-alpine-minirootfs
	tar xf alpine-minirootfs-3.13.0-aarch64.tar.gz -C aarch64-alpine-minirootfs

alpine-minirootfs-3.13.0-aarch64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v3.13/releases/aarch64/alpine-minirootfs-3.13.0-aarch64.tar.gz

ifneq ($(shell command -v alpine-make-rootfs),)
aarch64-alpine-3.14-chroot aarch64-alpine-edge-chroot:
aarch64-alpine-%-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-alpine-$*-rootfs
aarch64-alpine-%-chroot: | aarch64-alpine-%-rootfs
	bash iamroot-shell -c "chroot aarch64-alpine-$*-rootfs"

.PHONY: aarch64-alpine-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.14-rootfs aarch64-alpine-edge-rootfs

aarch64-alpine-3.14-rootfs: | aarch64-alpine-3.14-rootfs/bin/busybox
aarch64-alpine-edge-rootfs: | aarch64-alpine-edge-rootfs/bin/busybox

aarch64-alpine-%-rootfs/bin/busybox: | libiamroot-musl-aarch64.so.1 libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "APK_OPTS='--arch aarch64' alpine-make-rootfs aarch64-alpine-$*-rootfs --keys-dir /usr/share/apk/keys/aarch64 --mirror-uri http://nl.alpinelinux.org/alpine --branch $*"
endif
endif

qemu-system-x86_64-%: override CMDLINE += panic=5
qemu-system-x86_64-%: override CMDLINE += console=ttyS0
qemu-system-x86_64-%: override QEMUSYSTEMFLAGS += -enable-kvm -m 4G -machine q35 -smp 4 -cpu host
qemu-system-x86_64-%: override QEMUSYSTEMFLAGS += -nographic -serial mon:stdio
qemu-system-x86_64-%: override QEMUSYSTEMFLAGS += -kernel /boot/vmlinuz-linux
qemu-system-x86_64-%: override QEMUSYSTEMFLAGS += -initrd initrd-rootfs.cpio
qemu-system-x86_64-%: override QEMUSYSTEMFLAGS += -drive file=$*.ext4,if=virtio
qemu-system-x86_64-%: override QEMUSYSTEMFLAGS += -append "$(CMDLINE)"
qemu-system-x86_64-%: | %.ext4 initrd-rootfs.cpio
	qemu-system-x86_64 $(QEMUSYSTEMFLAGS)

ifneq ($(KVER),)
.PRECIOUS: %-rootfs/lib/modules/$(KVER) %-rootfs/usr/lib/modules/$(KVER)
%-rootfs/lib/modules/$(KVER): | %-rootfs
	rm -Rf $@.tmp $@
	mkdir -p $@.tmp
	rsync -a --include '*/' --include '*.ko*' --exclude '*' /usr/lib/modules/$(KVER)/. $@.tmp/.
	find $@.tmp -name "*.zst" -exec unzstd -q --rm {} \;
	mv $@.tmp $@
endif

initrd-rootfs.cpio: initrd-rootfs/init
initrd-rootfs.cpio: initrd-rootfs/bin/sh
initrd-rootfs.cpio: initrd-rootfs/etc/passwd
initrd-rootfs.cpio: initrd-rootfs/etc/group
initrd-rootfs.cpio: initrd-rootfs/etc/mdev.conf
ifneq ($(KVER),)
initrd-rootfs.cpio: initrd-rootfs/lib/modules/$(KVER)
initrd-rootfs.cpio: initrd-rootfs/lib/modules/$(KVER)/modules.dep
initrd-rootfs.cpio: initrd-rootfs/lib/modules/$(KVER)/modules.alias
initrd-rootfs.cpio: initrd-rootfs/lib/modules/$(KVER)/modules.symbols
endif

initrd-rootfs/init: tinird.sh | initrd-rootfs
	cp $< $@

initrd-rootfs/bin/sh: | initrd-rootfs/bin/busybox initrd-rootfs/bin
	ln -sf busybox $@

initrd-rootfs/bin/busybox: busybox-static | initrd-rootfs/bin
	cp $< $@

initrd-rootfs/etc/passwd: | initrd-rootfs/etc
	echo "root::0:0:root:/root:/bin/sh" >$@

initrd-rootfs/etc/group: | initrd-rootfs/etc
	echo "root:x:0:root" >$@

initrd-rootfs/etc/mdev.conf: | initrd-rootfs/etc
	echo '$$MODALIAS=.* root:root 660 @busybox modprobe "$$MODALIAS"' >$@

initrd-rootfs initrd-rootfs/bin initrd-rootfs/etc:
	mkdir -p $@

ifneq ($(KVER),)
initrd-rootfs/lib/modules/$(KVER)/modules.%: initrd-rootfs/bin/busybox | initrd-rootfs/lib/modules/$(KVER)
	initrd-rootfs/bin/busybox depmod -b initrd-rootfs $(KVER) $(F@)
endif

%.cpio:
	cd $* && find . | cpio -H newc -o -R root:root >$(CURDIR)/$@

ifneq ($(VMLINUX_KVER),)
vmlinux-%: override VMLINUXFLAGS+=panic=5
vmlinux-%: override VMLINUXFLAGS+=console=tty0 con0=fd:0,fd:1 con=none
vmlinux-%: override VMLINUXFLAGS+=mem=256M
vmlinux-%: override VMLINUXFLAGS+=rw
vmlinux-%: override VMLINUXFLAGS+=ubd0=$*.ext4
vmlinux-%: override VMLINUXFLAGS+=$(CMDLINE)
vmlinux-%: | %.ext4
	settings=$$(stty -g); \
	if ! vmlinux $(VMLINUXFLAGS); then \
		stty "$$settings"; \
		false; \
	fi; \
	stty "$$settings"
endif

.PRECIOUS: %.ext4
ifneq ($(KVER),)
MODULESDIRS += %-rootfs/usr/lib/modules/$(KVER)
endif
ifneq ($(VMLINUX_KVER),)
MODULESDIRS += %-rootfs/usr/lib/modules/$(VMLINUX_KVER)
endif
%.ext4: | libiamroot-linux-x86-64.so.2 %-rootfs $(MODULESDIRS)
	$(MAKE) $*-postrootfs
	rm -f $@.tmp
	fallocate --length 2G $@.tmp
	bash iamroot-shell -c "mkfs.ext4 -d $*-rootfs $@.tmp"
	mv $@.tmp $@

.PRECIOUS: %-rootfs/usr/lib/modules/$(KVER) %-rootfs/usr/lib/modules/$(VMLINUX_KVER)
%-rootfs/usr/lib/modules/$(KVER) %-rootfs/usr/lib/modules/$(VMLINUX_KVER): | libiamroot-linux-x86-64.so.2 %-rootfs
	rm -Rf $@.tmp $@
	mkdir -p $(@D)
	bash iamroot-shell -c "rsync -a /usr/lib/modules/$(@F)/. $@.tmp/."
	mv $@.tmp $@

.PHONY: %-postrootfs
%-postrootfs:

.PHONY: static-chroot
static-chroot: libiamroot-linux-x86-64.so.2 | static-rootfs
	bash iamroot-shell -c "chroot static-rootfs /bin/sh"

.PHONY: static-rootfs
static-rootfs: static-rootfs/usr/bin/sh
static-rootfs: static-rootfs/bin
static-rootfs: static-rootfs/root

static-rootfs/usr/bin/sh: PATH := $(CURDIR):$(PATH)
static-rootfs/usr/bin/sh: | busybox-static static-rootfs/usr/bin
	busybox-static --install $(@D)

static-rootfs/bin: | static-rootfs/usr/bin
	ln -sf usr/bin $@

static-rootfs/usr/bin static-rootfs/root:
	mkdir -p $@

busybox-static: busybox/busybox
	cp $< $@

.SILENT: busybox/busybox
busybox/busybox: busybox/.config
	$(MAKE) -C busybox CONFIG_STATIC=y CC=musl-gcc LD=musl-gcc

.SILENT: busybox/.config
busybox/.config: busybox/Makefile
	echo CONFIG_MODPROBE_SMALL=n >$@
	yes "" | $(MAKE) -C busybox oldconfig

.SILENT: busybox/Makefile
busybox/Makefile:
	wget -qO- https://www.busybox.net | \
	sed -n '/<li><b>.* -- BusyBox .* (stable)<\/b>/,/<\/li>/{/<p>/s,.*<a.*href="\(.*\)">BusyBox \(.*\)</a>.*,wget -qO- \1 | tar xvj \&\& ln -sf busybox-\2 busybox,p}' | \
	head -n 1 | \
	$(SHELL)

busybox_menuconfig:
busybox_%:
	$(MAKE) -C busybox $* CONFIG_STATIC=y CC=musl-gcc LD=musl-gcc

chroot-%:
	$(MAKE) mount-$*
	-sudo chroot mnt $(SHELL)
	$(MAKE) umount-$*

mount-%: | %.ext4 mnt
	sudo mount -oloop $*.ext4 mnt

umount-%: | mnt
	sudo umount mnt

mnt:
	mkdir -p $@

%-debug %-debug1:
	$(MAKE) $* IAMROOT_DEBUG=1

%-debug2:
	$(MAKE) $* IAMROOT_DEBUG=2

%-debug3:
	$(MAKE) $* IAMROOT_DEBUG=3

%:
	$(MAKE) -f Makefile $@
