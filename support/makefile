#
# Copyright 2021-2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

PREFIX ?= /usr/local
KVER ?= $(shell uname -r 2>/dev/null)
VMLINUX_KVER ?= $(shell vmlinux --version 2>/dev/null)

IAMROOT_LIB = $(CURDIR)/x86_64/libiamroot-linux-x86-64.so.2:libdl.so.2
export IAMROOT_LIB

IAMROOT_LIB_LINUX_2 = $(CURDIR)/i686/libiamroot-linux.so.2
export IAMROOT_LIB_LINUX_2

IAMROOT_LIB_LINUX_X86_64_2 = $(CURDIR)/x86_64/libiamroot-linux-x86-64.so.2
export IAMROOT_LIB_LINUX_X86_64_2

IAMROOT_LIB_LINUX_3 = $(CURDIR)/arm/libiamroot-linux.so.3
export IAMROOT_LIB_LINUX_3

IAMROOT_LIB_LINUX_ARMHF_3 = $(CURDIR)/armhf/libiamroot-linux-armhf.so.3
export IAMROOT_LIB_LINUX_ARMHF_3

IAMROOT_LIB_LINUX_AARCH64_1 = $(CURDIR)/aarch64/libiamroot-linux-aarch64.so.1
export IAMROOT_LIB_LINUX_AARCH64_1

IAMROOT_LIB_MUSL_X86_64_1 = $(CURDIR)/x86_64/libiamroot-musl-x86_64.so.1
export IAMROOT_LIB_MUSL_X86_64_1

IAMROOT_LIB_MUSL_AARCH64_1 = $(CURDIR)/aarch64/libiamroot-musl-aarch64.so.1
export IAMROOT_LIB_MUSL_AARCH64_1

IAMROOT_EXEC = $(CURDIR)/exec.sh
export IAMROOT_EXEC

IAMROOT_EXEC_IGNORE = ldd|mountpoint
export IAMROOT_EXEC_IGNORE

-include local.mk

MAKEFLAGS += --no-print-directory

.PHONY: all
all:

define libiamroot_so =

all: $(1)/libiamroot-$(2).so.$(3)
ci: $(1)/libiamroot-$(2).so.$(3)

.PRECIOUS: $(1)/libiamroot-$(2).so.$(3)
$(1)/libiamroot-$(2).so.$(3): output-$(1)-$(2)/libiamroot.so

output-$(1)-$(2)/libiamroot.so:

install: install-exec-$(1)-$(2).$(3)

.PHONY: install-exec-$(1)-$(2).$(3)
install-exec-$(1)-$(2).$(3):
	install -D -m 755 $(1)/libiamroot-$(2).so.$(3) $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so.$(3)
	ln -sf libiamroot-$(2).so.$(3) $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so

uninstall: uninstall-$(1)-$(2).$(3)

.PHONY: uninstall-$(1)-$(2).$(3)
uninstall-$(1)-$(2).$(3):
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so.$(3)
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so

clean: clean-$(1)-$(2).$(3)

.PHONY: clean-$(1)-$(2).$(3)
clean-$(1)-$(2).$(3):
	rm -Rf output-$(1)-$(2)/
	rm -Rf $(1)/
endef

$(eval $(call libiamroot_so,x86_64,linux-x86-64,2))

output-i686-linux/libiamroot.so: export CC += -m32
output-i686-linux/libiamroot.so: export CFLAGS += -fno-stack-protector
$(eval $(call libiamroot_so,i686,linux,2))

ifneq ($(shell command -v arm-unknown-linux-gnueabi-gcc 2>/dev/null),)
output-arm-linux/libiamroot.so: export CC = arm-unknown-linux-gnueabi-gcc
$(eval $(call libiamroot_so,arm,linux,3))
endif

ifneq ($(shell command -v armv7l-unknown-linux-gnueabihf-gcc 2>/dev/null),)
output-armhf-linux-armhf/libiamroot.so: export CC = armv7l-unknown-linux-gnueabihf-gcc
$(eval $(call libiamroot_so,armhf,linux-armhf,3))
endif

ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
aarch64/libiamroot-linux-aarch64.so.1: export CC = aarch64-linux-gnu-gcc
$(eval $(call libiamroot_so,aarch64,linux-aarch64,1))
endif

ifneq ($(shell command -v musl-gcc 2>/dev/null),)
x86_64/libiamroot-musl-x86_64.so.1: export CC = musl-gcc
$(eval $(call libiamroot_so,x86_64,musl-x86_64,1))
endif

ifneq ($(shell command -v aarch64-linux-musl-gcc 2>/dev/null),)
aarch64/libiamroot-musl-aarch64.so.1: export CC = aarch64-linux-musl-gcc
$(eval $(call libiamroot_so,aarch64,musl-aarch64,1))
endif

ifdef CLANG
ifneq ($(shell command -v clang 2>/dev/null),)
all: clang/libiamroot-linux-x86-64.so.2

.PRECIOUS: clang/libiamroot-linux-x86_64.so.2
clang/libiamroot-linux-x86-64.so.2: output-clang-linux-x86-64/libiamroot.so

output-clang-linux-x86-64/libiamroot.so:

clang/libiamroot-linux-x86-64.so.2:
clang/libiamroot-linux-x86-64.so.2: export CC = clang

clean: clean-clang-linux-x86-64.2

.PHONY: clean-clang-linux-x86-64.2
clean-clang-linux-x86-64.2:
	rm -Rf output-clang-x86-64/
	rm -Rf clang/
endif
endif

.PRECIOUS: output-%/libiamroot.so
output-%/libiamroot.so: $(wildcard *.c) | output-%
	$(MAKE) -f $(CURDIR)/Makefile -C output-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: output-%
output-%:
	mkdir $@

%.so.1 %.so.2 %.so.3:
	install -D -m755 $< $@

.NOTPARALLEL: fixme-rootfs
.PHONY: fixme-rootfs
fixme-rootfs:

.NOTPARALLEL: rootfs
.PHONY: rootfs
rootfs: i686-rootfs

.PHONY: extra-rootfs
extra-rootfs:

.PHONY: i686-rootfs
i686-rootfs:

.PHONY: aarch64-rootfs
aarch64-rootfs:

.PHONY: arm-rootfs
arm-rootfs:

.PHONY: test
test: | alpine-minirootfs
	$(MAKE) -f Makefile $@

.PHONY: coverage
coverage: gcov/index.html

.PHONY: gcov/index.html
gcov/index.html:
	mkdir -p $(@D)
	gcovr --html-details --html-title iamroot -s -r $(CURDIR) -o $@ output-x86_64-linux-x86-64/

.PHONY: cobertura.xml
cobertura.xml:
	gcovr --cobertura -s -r $(CURDIR) -o $@ output-x86_64-linux-x86-64/

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf *.gcda *.gcno
	rm -f *.ext4 *.cpio
	rm -f *-rootfs.log
	rm -Rf *-rootfs/

ifneq ($(shell command -v pacstrap 2>/dev/null),)
test: arch-test

.PHONY: arch-test
arch-test: | arch-rootfs/usr/bin/shebang.sh
arch-test: | arch-rootfs/usr/bin/shebang-arg.sh
arch-test: x86_64/libiamroot-linux-x86-64.so.2 | arch-rootfs
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

arch-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda
arch-rootfs/etc/machine-id: export EUID = 0
arch-rootfs/etc/machine-id: | arch-rootfs/etc/pacman.d/gnupg x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacstrap -GMC support/x86_64-pacman.conf arch-rootfs"

arch-rootfs/etc/pacman.d/gnupg: | x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacman-key --conf support/x86_64-pacman.conf --gpgdir $@.tmp --init"
	bash iamroot-shell -c "pacman-key --conf support/x86_64-pacman.conf --gpgdir $@.tmp --populate archlinux"
	mv $@.tmp $@

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
i686-arch-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/output-i686-linux/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda
i686-arch-rootfs/etc/machine-id: export EUID = 0
i686-arch-rootfs/etc/machine-id: | i686-arch-rootfs/etc/pacman.d/gnupg i686/libiamroot-linux.so.2 x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacstrap -GMC support/i686-pacman.conf i686-arch-rootfs"

i686-arch-rootfs/etc/pacman.d/gnupg: archlinux32.gpg
i686-arch-rootfs/etc/pacman.d/gnupg: archlinux32-trusted
i686-arch-rootfs/etc/pacman.d/gnupg: | x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacman-key --conf support/i686-pacman.conf --gpgdir $@.tmp --init"
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --import archlinux32.gpg"
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --import-ownertrust archlinux32-trusted"
	while IFS=: read key_id _; do \
		bash iamroot-shell -c "printf 'y\ny\n' | LANG=C gpg --homedir $@.tmp --no-permission-warning --command-fd 0 --quiet --batch --lsign-key $$key_id"; \
	done <archlinux32-trusted
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --batch --check-trustdb"
	mv $@.tmp $@

qemu-system-x86_64-arch:

ifneq ($(VMLINUX_KVER),)
vmlinux-arch:
endif

arch.ext4:

arch-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i arch-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i arch-rootfs/etc/shadow
	mkdir -p arch-rootfs/var/lib/systemd/linger
	rm -f arch-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot arch-rootfs systemctl enable getty@tty0.service"

chroot-arch:

mount-arch:

umount-arch:

.PHONY: manjaro-chroot
manjaro-chroot: | manjaro-rootfs
	bash iamroot-shell -c "chroot manjaro-rootfs"

extra-rootfs: manjaro-rootfs

.PHONY: manjaro-rootfs
manjaro-rootfs: | manjaro-rootfs/etc/machine-id

manjaro-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda
manjaro-rootfs/etc/machine-id: export EUID = 0
manjaro-rootfs/etc/machine-id: | manjaro-rootfs/etc/pacman.d/gnupg x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacstrap -GMC support/x86_64-manjaro-stable-pacman.conf manjaro-rootfs --gpgdir $(CURDIR)/manjaro-rootfs/etc/pacman.d/gnupg base"

manjaro-rootfs/etc/pacman.d/gnupg: manjaro.gpg
manjaro-rootfs/etc/pacman.d/gnupg: manjaro-trusted
manjaro-rootfs/etc/pacman.d/gnupg: | x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacman-key --conf support/x86_64-manjaro-stable-pacman.conf --gpgdir $@.tmp --init"
	bash iamroot-shell -c "pacman-key --conf support/x86_64-manjaro-stable-pacman.conf --gpgdir $@.tmp --populate archlinux"
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --import manjaro.gpg"
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --import-ownertrust manjaro-trusted"
	while IFS=: read key_id _; do \
		bash iamroot-shell -c "printf 'y\ny\n' | LANG=C gpg --homedir $@.tmp --no-permission-warning --command-fd 0 --quiet --batch --lsign-key $$key_id"; \
	done <manjaro-trusted
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --batch --check-trustdb"
	mv $@.tmp $@

qemu-system-x86_64-manjaro:

ifneq ($(VMLINUX_KVER),)
vmlinux-manjaro:
endif

manjaro.ext4:

manjaro-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i manjaro-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i manjaro-rootfs/etc/shadow
	mkdir -p manjaro-rootfs/var/lib/systemd/linger
	rm -f manjaro-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot manjaro-rootfs systemctl enable getty@tty0.service"

chroot-manjaro:

mount-manjaro:

umount-manjaro:
endif

ifneq ($(shell command -v debootstrap 2>/dev/null),)
debian-oldoldstable-chroot:
debian-oldstable-chroot:
debian-stable-chroot:
debian-testing-chroot:
debian-unstable-chroot:
debian-%-chroot: export IAMROOT_LIBRARY_PATH = /usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib
debian-%-chroot: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2:/lib/x86_64-linux-gnu/libpthread.so.0
debian-%-chroot: | debian-%-rootfs
	bash iamroot-shell -c "chroot debian-$*-rootfs"

rootfs: debian-rootfs

.NOTPARALLEL: debian-rootfs
.PHONY: debian-rootfs
debian-rootfs: debian-oldoldstable-rootfs 
debian-rootfs: debian-oldstable-rootfs 
debian-rootfs: debian-stable-rootfs 
debian-rootfs: debian-testing-rootfs 
debian-rootfs: debian-unstable-rootfs

debian-oldoldstable-rootfs: | debian-oldoldstable-rootfs/etc/machine-id
debian-oldstable-rootfs: | debian-oldstable-rootfs/etc/machine-id
debian-stable-rootfs: | debian-stable-rootfs/etc/machine-id
debian-testing-rootfs: | debian-testing-rootfs/etc/machine-id
debian-unstable-rootfs: | debian-unstable-rootfs/etc/machine-id

debian-oldoldstable-rootfs/etc/machine-id:
debian-oldoldstable-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda
debian-oldstable-rootfs/etc/machine-id:
debian-oldstable-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda
debian-stable-rootfs/etc/machine-id:
debian-stable-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda

debian-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib
debian-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2:/lib/x86_64-linux-gnu/libpthread.so.0
debian-%-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update
debian-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda
debian-%-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://deb.debian.org/debian
# System has not been booted with systemd as init system (PID 1). Can't operate.
# Failed to connect to bus: Host is down
# invoke-rc.d: could not determine current runlevel
debian-%-rootfs/etc/machine-id: export SYSTEMD_OFFLINE = 1
# debconf: PERL_DL_NONLAZY is not set, if debconf is running from a preinst script, this is not safe
debian-%-rootfs/etc/machine-id: export PERL_DL_NONLAZY = 1
debian-%-rootfs/etc/machine-id: export DEBOOTSTRAPFLAGS ?= --no-check-gpg
debian-%-rootfs/etc/machine-id: | x86_64/libiamroot-linux-x86-64.so.2
	mkdir -p debian-$*-rootfs
	bash iamroot-shell -c "debootstrap --keep-debootstrap-dir $(DEBOOTSTRAPFLAGS) $* debian-$*-rootfs $(DEBOOTSTRAP_MIRROR)"
	cat debian-$*-rootfs/debootstrap/debootstrap.log
	rm -Rf debian-$*-rootfs/debootstrap/

qemu-system-x86_64-debian-oldoldstable:
qemu-system-x86_64-debian-oldoldstable: override CMDLINE += rw
qemu-system-x86_64-debian-oldstable:
qemu-system-x86_64-debian-oldstable: override CMDLINE += rw
qemu-system-x86_64-debian-stable:
qemu-system-x86_64-debian-stable: override CMDLINE += rw
qemu-system-x86_64-debian-testing:
qemu-system-x86_64-debian-testing: override CMDLINE += rw
qemu-system-x86_64-debian-unstable:
qemu-system-x86_64-debian-unstable: override CMDLINE += rw

ifneq ($(VMLINUX_KVER),)
vmlinux-debian-oldoldstable:
vmlinux-debian-oldstable:
vmlinux-debian-stable:
vmlinux-debian-testing:
vmlinux-debian-unstable:
endif

debian-oldoldstable.ext4:
debian-oldstable.ext4:
debian-stable.ext4:
debian-stable.ext4:
debian-testing.ext4:
debian-unstable.ext4:

debian-oldoldstable-postrootfs: export IAMROOT_LIBRARY_PATH = /usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib
debian-oldoldstable-postrootfs: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2
debian-oldoldstable-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
debian-oldoldstable-postrootfs:
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i debian-oldoldstable-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i debian-oldoldstable-rootfs/etc/shadow
	rm -f debian-oldoldstable-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot debian-oldoldstable-rootfs systemctl enable getty@tty0.service"
	bash iamroot-shell -c "chroot debian-oldoldstable-rootfs pam-auth-update"

debian-oldstable-postrootfs:
debian-stable-postrootfs:
debian-testing-postrootfs:
debian-unstable-postrootfs:
debian-%-postrootfs: export IAMROOT_LIBRARY_PATH = /usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib
debian-%-postrootfs: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2
debian-%-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i debian-$*-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i debian-$*-rootfs/etc/shadow
	rm -f debian-$*-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot debian-$*-rootfs systemctl enable getty@tty0.service"
	rm -f debian-$*-rootfs/etc/systemd/system/multi-user.target.wants/sshd.service
	bash iamroot-shell -c "chroot debian-$*-rootfs systemctl disable sshd.service"
	bash iamroot-shell -c "chroot debian-$*-rootfs pam-auth-update"

chroot-debian-oldoldstable:
chroot-debian-oldstable:
chroot-debian-stable:
chroot-debian-stable:
chroot-debian-testing:
chroot-debian-unstable:

mount-debian-oldoldstable:
mount-debian-oldstable:
mount-debian-stable:
mount-debian-testing:
mount-debian-unstable:

umount-debian-oldoldstable:
umount-debian-oldstable:
umount-debian-stable:
umount-debian-testing:
umount-debian-unstable:

ubuntu-bionic-chroot:
ubuntu-focal-chroot:
ubuntu-impish-chroot:
ubuntu-jammy-chroot:
ubuntu-%-chroot: export IAMROOT_LIBRARY_PATH = /usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib
ubuntu-%-chroot: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2:/lib/x86_64-linux-gnu/libpthread.so.0
ubuntu-%-chroot: | ubuntu-%-rootfs
	bash iamroot-shell -c "chroot ubuntu-$*-rootfs"

.NOTPARALLEL: ubuntu-rootfs
.PHONY: ubuntu-rootfs
ubuntu-rootfs: ubuntu-bionic-rootfs
ubuntu-rootfs: ubuntu-focal-rootfs
ubuntu-rootfs: ubuntu-impish-rootfs
ubuntu-rootfs: ubuntu-jammy-rootfs

ubuntu-bionic-rootfs: | ubuntu-bionic-rootfs/etc/machine-id
ubuntu-focal-rootfs: | ubuntu-focal-rootfs/etc/machine-id
ubuntu-impish-rootfs: | ubuntu-impish-rootfs/etc/machine-id
ubuntu-jammy-rootfs: | ubuntu-jammy-rootfs/etc/machine-id

ubuntu-bionic-rootfs/etc/machine-id:
ubuntu-focal-rootfs/etc/machine-id:
ubuntu-impish-rootfs/etc/machine-id:
ubuntu-jammy-rootfs/etc/machine-id:

ubuntu-bionic-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|/var/lib/dpkg/info/initramfs-tools.postinst
ubuntu-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib
ubuntu-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2:/lib/x86_64-linux-gnu/libpthread.so.0
ubuntu-%-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update
ubuntu-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda
ubuntu-%-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
ubuntu-%-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
# System has not been booted with systemd as init system (PID 1). Can't operate.
# Failed to connect to bus: Host is down
# invoke-rc.d: could not determine current runlevel
ubuntu-%-rootfs/etc/machine-id: export SYSTEMD_OFFLINE = 1
# debconf: PERL_DL_NONLAZY is not set, if debconf is running from a preinst script, this is not safe
ubuntu-%-rootfs/etc/machine-id: export PERL_DL_NONLAZY = 1
ubuntu-%-rootfs/etc/machine-id: export DEBOOTSTRAPFLAGS ?= --no-check-gpg
ubuntu-%-rootfs/etc/machine-id: | x86_64/libiamroot-linux-x86-64.so.2
	mkdir -p ubuntu-$*-rootfs
	bash iamroot-shell -c "debootstrap --keep-debootstrap-dir $(DEBOOTSTRAPFLAGS) $* ubuntu-$*-rootfs $(DEBOOTSTRAP_MIRROR)"
	cat ubuntu-$*-rootfs/debootstrap/debootstrap.log
	rm -Rf ubuntu-$*-rootfs/debootstrap/

qemu-system-x86_64-ubuntu-bionic:
qemu-system-x86_64-ubuntu-bionic: override CMDLINE += rw
qemu-system-x86_64-ubuntu-focal:
qemu-system-x86_64-ubuntu-focal: override CMDLINE += rw
qemu-system-x86_64-ubuntu-impish:
qemu-system-x86_64-ubuntu-impish: override CMDLINE += rw
qemu-system-x86_64-ubuntu-jammy:
qemu-system-x86_64-ubuntu-jammy: override CMDLINE += rw

ifneq ($(VMLINUX_KVER),)
vmlinux-ubuntu-bionic:
vmlinux-ubuntu-focal:
vmlinux-ubuntu-impish:
vmlinux-ubuntu-jammy:
endif

ubuntu-bionic.ext4:
ubuntu-focal.ext4:
ubuntu-impish.ext4:
ubuntu-jammy.ext4:

ubuntu-bionic-postrootfs:
ubuntu-focal-postrootfs:
ubuntu-impish-postrootfs:
ubuntu-jammy-postrootfs:
ubuntu-%-postrootfs: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2
ubuntu-%-postrootfs: export IAMROOT_LIBRARY_PATH = /usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib
ubuntu-%-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i ubuntu-$*-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i ubuntu-$*-rootfs/etc/shadow
	rm -f ubuntu-$*-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot ubuntu-$*-rootfs systemctl enable getty@tty0.service"
	rm -f ubuntu-$*-rootfs/etc/systemd/system/multi-user.target.wants/sshd.service
	bash iamroot-shell -c "chroot ubuntu-$*-rootfs systemctl disable sshd.service"
	bash iamroot-shell -c "chroot ubuntu-$*-rootfs pam-auth-update"

chroot-ubuntu-bionic:
chroot-ubuntu-focal:
chroot-ubuntu-impish:
chroot-ubuntu-jammy:

mount-ubuntu-bionic:
mount-ubuntu-focal:
mount-ubuntu-impish:
mount-ubuntu-jammy:

umount-ubuntu-bionic:
umount-ubuntu-focal:
umount-ubuntu-impish:
umount-ubuntu-jammy:
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
fedora-33-chroot:
fedora-34-chroot:
fedora-35-chroot:
fedora-36-chroot:
fedora-%-chroot: export IAMROOT_LIBRARY_PATH = /usr/lib64:/lib64
fedora-%-chroot: | fedora-%-rootfs
	bash iamroot-shell -c "chroot fedora-$*-rootfs"

rootfs: fedora-rootfs

.NOTPARALLEL: fedora-rootfs
.PHONY: fedora-rootfs
fedora-rootfs: fedora-33-rootfs
fedora-rootfs: fedora-34-rootfs
fedora-rootfs: fedora-35-rootfs
fedora-rootfs: fedora-36-rootfs

fedora-33-rootfs: | fedora-33-rootfs/etc/machine-id
fedora-34-rootfs: | fedora-34-rootfs/etc/machine-id
fedora-35-rootfs: | fedora-35-rootfs/etc/machine-id
fedora-36-rootfs: | fedora-36-rootfs/etc/machine-id

fedora-33-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib64/ldb/modules/ldb/tdb.so:/usr/lib64/ldb/modules/ldb/mdb.so:/usr/lib64/ldb/modules/ldb/ldb.so
fedora-33-rootfs/etc/machine-id:
fedora-34-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib64/ldb/modules/ldb/tdb.so:/usr/lib64/ldb/modules/ldb/mdb.so:/usr/lib64/ldb/modules/ldb/ldb.so
fedora-34-rootfs/etc/machine-id:

fedora-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib64/ldb:/usr/lib64:/lib64
fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_ALLOW = ^/var/run/(console|sepermit|faillock)
fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda|^$(CURDIR)/fedora-$*-rootfs/var/log/dnf.rpm.log
fedora-%-rootfs/etc/machine-id: | x86_64/libiamroot-linux-x86-64.so.2
	install -D -m644 support/fedora.repo fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --releasever $* --assumeyes --installroot $(CURDIR)/fedora-$*-rootfs group install minimal-environment"
	rm -f fedora-$*-rootfs/etc/distro.repos.d/fedora.repo

qemu-system-x86_64-fedora-33:
qemu-system-x86_64-fedora-33: override CMDLINE += rw
qemu-system-x86_64-fedora-34:
qemu-system-x86_64-fedora-34: override CMDLINE += rw
qemu-system-x86_64-fedora-35:
qemu-system-x86_64-fedora-35: override CMDLINE += rw
qemu-system-x86_64-fedora-36:
qemu-system-x86_64-fedora-36: override CMDLINE += rw

ifneq ($(VMLINUX_KVER),)
vmlinux-fedora-33:
vmlinux-fedora-34:
vmlinux-fedora-35:
vmlinux-fedora-36:
endif

fedora-33.ext4:
fedora-34.ext4:
fedora-35.ext4:
fedora-36.ext4:

fedora-33-postrootfs:
fedora-34-postrootfs:
fedora-35-postrootfs:
fedora-36-postrootfs:
fedora-%-postrootfs: export IAMROOT_LIBRARY_PATH = /usr/lib64:/lib64
fedora-%-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i fedora-$*-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i fedora-$*-rootfs/etc/shadow
	touch fedora-$*-rootfs/etc/systemd/zram-generator.conf
	mkdir -p fedora-$*-rootfs/var/lib/systemd/linger
	rm -f fedora-$*-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot fedora-$*-rootfs systemctl enable getty@tty0.service"
	rm -f fedora-$*-rootfs/etc/systemd/system/multi-user.target.wants/sshd.service
	bash iamroot-shell -c "chroot fedora-$*-rootfs systemctl disable sshd.service"

chroot-fedora-33:
chroot-fedora-34:
chroot-fedora-35:
chroot-fedora-36:

mount-fedora-33:
mount-fedora-34:
mount-fedora-35:
mount-fedora-36:

umount-fedora-33:
umount-fedora-34:
umount-fedora-35:
umount-fedora-36:
endif

ifneq ($(shell command -v zypper 2>/dev/null),)
opensuse-leaf-chroot: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib64/libc.so.6:/lib64/libdl.so.2
opensuse-leaf-chroot:
opensuse-tumbleweed-chroot:
opensuse-%-chroot: export IAMROOT_LIBRARY_PATH = /usr/lib64:/lib64
opensuse-%-chroot: | opensuse-%-rootfs
	bash iamroot-shell -c "chroot opensuse-$*-rootfs"

extra-rootfs: opensuse-rootfs

.NOTPARALLEL: opensuse-rootfs
.PHONY: opensuse-rootfs
opensuse-rootfs: | opensuse-tumbleweed-rootfs

opensuse-leaf-rootfs: | opensuse-leaf-rootfs/etc/machine-id
opensuse-tumbleweed-rootfs: | opensuse-tumbleweed-rootfs/etc/machine-id

opensuse-leaf-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib64/libc.so.6:/lib64/libdl.so.2
opensuse-leaf-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|/usr/bin/chkstat|/usr/sbin/update-ca-certificates
opensuse-leaf-rootfs/etc/machine-id:

opensuse-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib64:/lib64
opensuse-%-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|/usr/bin/chkstat
opensuse-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-x86_64-linux-x86-64/.*\.gcda
opensuse-%-rootfs/etc/machine-id: | x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "zypper --root $(CURDIR)/opensuse-$*-rootfs addrepo --no-gpgcheck support/$*-repo-oss.repo"
	bash iamroot-shell -c "zypper --root $(CURDIR)/opensuse-$*-rootfs --non-interactive --no-gpg-checks install patterns-base-minimal_base zypper systemd"

qemu-system-x86_64-opensuse-leaf:
qemu-system-x86_64-opensuse-leaf: override CMDLINE += rw init=/usr/lib/systemd/systemd
qemu-system-x86_64-opensuse-tumbleweed:
qemu-system-x86_64-opensuse-tumbleweed: override CMDLINE += rw

ifneq ($(VMLINUX_KVER),)
vmlinux-opensuse-leaf:
vmlinux-opensuse-tumbleweed:
endif

opensuse-leaf.ext4:
opensuse-tumbleweed.ext4:

opensuse-leaf-postrootfs: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib64/libc.so.6:/lib64/libdl.so.2
opensuse-leaf-postrootfs:
opensuse-tumbleweed-postrootfs:
opensuse-%-postrootfs: export IAMROOT_LIBRARY_PATH = /usr/lib64:/lib64
opensuse-%-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i opensuse-$*-rootfs/etc/passwd
	sed -e '/^root:\*:/s,^root:\*:,root:x:,' \
	    -i opensuse-$*-rootfs/etc/shadow
	bash iamroot-shell -c "chroot opensuse-$*-rootfs pam-config -a --nullok"
	mkdir -p arch-rootfs/var/lib/systemd/linger
	rm -f opensuse-$*-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot opensuse-$*-rootfs systemctl enable getty@tty0.service"
	rm -f opensuse-$*-rootfs/etc/systemd/system/getty.target.wants/getty@ttyS0.service
	bash iamroot-shell -c "chroot opensuse-$*-rootfs systemctl enable getty@ttyS0.service"

chroot-opensuse-leaf:
chroot-opensuse-tumbleweed:

mount-opensuse-leaf:
mount-opensuse-tumbleweed:

umount-opensuse-leaf:
umount-opensuse-tumbleweed:
endif

ifneq ($(shell command -v musl-gcc 2>/dev/null),)
.PHONY: alpine-test
alpine-test: | alpine-minirootfs/usr/bin/shebang.sh
alpine-test: | alpine-minirootfs/usr/bin/shebang-arg.sh
alpine-test: | alpine-minirootfs/usr/bin/shebang-busybox.sh
alpine-test: x86_64/libiamroot-musl-x86_64.so.1 x86_64/libiamroot-linux-x86-64.so.2 | alpine-minirootfs
	bash iamroot-shell -c "chroot alpine-minirootfs pwd" | tee /dev/stderr | grep -q "^/\$$"
	bash iamroot-shell -c "chroot alpine-minirootfs cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell -c "chroot alpine-minirootfs chroot . cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell -c "chroot alpine-minirootfs /bin/busybox"
	bash iamroot-shell -c "chroot alpine-minirootfs shebang.sh one two three"
	bash iamroot-shell -c "chroot alpine-minirootfs shebang-arg.sh one two three"
	bash iamroot-shell -c "chroot alpine-minirootfs shebang-busybox.sh one two three"
	bash iamroot-shell -c "chroot alpine-minirootfs /lib/ld-musl-x86_64.so.1 --preload "$$PWD/x86_64/libiamroot-musl-x86_64.so.1" bin/busybox"

rootfs: alpine-rootfs

.PHONY: mini-chroot
mini-chroot: x86_64/libiamroot-musl-x86_64.so.1 x86_64/libiamroot-linux-x86-64.so.2 | alpine-minirootfs
	bash iamroot-shell -c "chroot alpine-minirootfs /bin/sh"

alpine-minirootfs/usr/bin/%: support/% | alpine-minirootfs
	cp $< $@

.PHONY: alpine-minirootfs
alpine-minirootfs: | alpine-minirootfs/bin/busybox

alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.16.0-x86_64.tar.gz
	mkdir -p alpine-minirootfs
	tar xf alpine-minirootfs-3.16.0-x86_64.tar.gz -C alpine-minirootfs

alpine-minirootfs-3.16.0-x86_64.tar.gz:
alpine-minirootfs-%-x86_64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v$(basename $*)/releases/x86_64/alpine-minirootfs-$*-x86_64.tar.gz

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
alpine-3.14-chroot:
alpine-3.15-chroot:
alpine-3.16-chroot:
alpine-edge-chroot:
alpine-%-chroot: | alpine-%-rootfs
	bash iamroot-shell -c "chroot alpine-$*-rootfs /bin/ash"

.NOTPARALLEL: alpine-rootfs
.PHONY: alpine-rootfs
alpine-rootfs: alpine-3.14-rootfs
alpine-rootfs: alpine-3.15-rootfs
alpine-rootfs: alpine-3.16-rootfs
alpine-rootfs: alpine-edge-rootfs

alpine-3.14-rootfs: | alpine-3.14-rootfs/bin/busybox
alpine-3.15-rootfs: | alpine-3.15-rootfs/bin/busybox
alpine-3.16-rootfs: | alpine-3.16-rootfs/bin/busybox
alpine-edge-rootfs: | alpine-edge-rootfs/bin/busybox

alpine-%-rootfs/bin/busybox: | x86_64/libiamroot-musl-x86_64.so.1 x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "alpine-make-rootfs alpine-$*-rootfs --keys-dir /usr/share/apk/keys/x86_64 --mirror-uri http://nl.alpinelinux.org/alpine --branch $*"

qemu-system-x86_64-alpine-3.14:
qemu-system-x86_64-alpine-3.15:
qemu-system-x86_64-alpine-3.16:
qemu-system-x86_64-alpine-edge:

ifneq ($(VMLINUX_KVER),)
vmlinux-alpine-3.14:
vmlinux-alpine-3.15:
vmlinux-alpine-3.16:
vmlinux-alpine-edge:
endif

alpine-3.14.ext4:
alpine-3.15.ext4:
alpine-3.16.ext4:
alpine-edge.ext4:

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
chroot-alpine-3.14:
chroot-alpine-3.15:
chroot-alpine-3.16:
chroot-alpine-edge:

mount-alpine-3.14:
mount-alpine-3.15:
mount-alpine-3.16:
mount-alpine-edge:

umount-alpine-3.14:
umount-alpine-3.15:
umount-alpine-3.16:
umount-alpine-edge:
endif

gcompat/ld-linux.so.2 gcompat/libgcompat.so.0: | gcompat/Makefile
	$(MAKE) -C gcompat CC=musl-gcc

gcompat/Makefile:
	git clone https://code.foxkit.us/adelie/gcompat.git

clean: clean-gcompat

.PHONY: clean-gcompat
clean-gcompat:
	-$(MAKE) -C gcompat clean

ifdef COVERAGE
IAMROOT_LIB_MUSL_X86_64_1 += :$(CURDIR)/gcompat/libgcompat.so.0
x86_64/libiamroot-musl-x86_64.so.1: | gcompat/libgcompat.so.0
endif
endif

ifneq ($(shell command -v pacstrap 2>/dev/null),)
ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
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
aarch64-arch-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-aarch64-linux-aarch64/.*\.gcda
aarch64-arch-rootfs/etc/machine-id: export EUID = 0
aarch64-arch-rootfs/etc/machine-id: | aarch64-arch-rootfs/etc/pacman.d/gnupg aarch64/libiamroot-linux-aarch64.so.1 x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacstrap -GMC support/aarch64-pacman.conf aarch64-arch-rootfs"

aarch64-arch-rootfs/etc/pacman.d/gnupg: archlinuxarm.gpg
aarch64-arch-rootfs/etc/pacman.d/gnupg: archlinuxarm-trusted
aarch64-arch-rootfs/etc/pacman.d/gnupg: | x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacman-key --conf support/aarch64-pacman.conf --gpgdir $@.tmp --init"
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --import archlinuxarm.gpg"
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --import-ownertrust archlinuxarm-trusted"
	while IFS=: read key_id _; do \
		bash iamroot-shell -c "printf 'y\ny\n' | LANG=C gpg --homedir $@.tmp --no-permission-warning --command-fd 0 --quiet --batch --lsign-key $$key_id"; \
	done <archlinuxarm-trusted
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --batch --check-trustdb"
	mv $@.tmp $@
endif

ifneq ($(shell command -v armv7l-unknown-linux-gnueabihf-gcc 2>/dev/null),)
.PHONY: armv7h-arch-chroot
armv7h-arch-chroot: export QEMU_LD_PREFIX = $(CURDIR)/armv7h-arch-rootfs
armv7h-arch-chroot: export IAMROOT_LD_PRELOAD_LINUX_ARMHF_3 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
armv7h-arch-chroot: | armv7h-arch-rootfs
	bash iamroot-shell -c "chroot armv7h-arch-rootfs"

arm-rootfs: armv7h-arch-rootfs

.PHONY: armv7h-arch-rootfs
armv7h-arch-rootfs: export QEMU_LD_PREFIX = $(CURDIR)/armv7h-arch-rootfs
armv7h-arch-rootfs: | armv7h-arch-rootfs/etc/machine-id

armv7h-arch-rootfs/etc/machine-id: export QEMU_LD_PREFIX = $(CURDIR)/armv7h-arch-rootfs
armv7h-arch-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_ARMHF_3 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
armv7h-arch-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-armhf-linux-armhf/.*\.gcda
armv7h-arch-rootfs/etc/machine-id: export EUID = 0
armv7h-arch-rootfs/etc/machine-id: | armv7h-arch-rootfs/etc/pacman.d/gnupg armhf/libiamroot-linux-armhf.so.3 x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacstrap -GMC support/armv7h-pacman.conf armv7h-arch-rootfs"

armv7h-arch-rootfs/etc/pacman.d/gnupg: archlinuxarm.gpg
armv7h-arch-rootfs/etc/pacman.d/gnupg: archlinuxarm-trusted
armv7h-arch-rootfs/etc/pacman.d/gnupg: | x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "pacman-key --conf support/armv7h-pacman.conf --gpgdir $@.tmp --init"
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --import archlinuxarm.gpg"
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --import-ownertrust archlinuxarm-trusted"
	while IFS=: read key_id _; do \
		bash iamroot-shell -c "printf 'y\ny\n' | LANG=C gpg --homedir $@.tmp --no-permission-warning --command-fd 0 --quiet --batch --lsign-key $$key_id"; \
	done <archlinuxarm-trusted
	bash iamroot-shell -c "gpg --homedir $@.tmp --no-permission-warning --batch --check-trustdb"
	mv $@.tmp $@
endif
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
aarch64-fedora-33-chroot:
aarch64-fedora-34-chroot:
aarch64-fedora-35-chroot:
aarch64-fedora-36-chroot:
aarch64-fedora-%-chroot: export IAMROOT_LIBRARY_PATH = /usr/lib64:/lib64
aarch64-fedora-%-chroot: | aarch64-fedora-%-rootfs
	bash iamroot-shell -c "chroot aarch64-fedora-$*-rootfs"

aarch64-rootfs: aarch64-fedora-rootfs

.PHONY: aarch64-fedora-rootfs
aarch64-fedora-rootfs: aarch64-fedora-33-rootfs
aarch64-fedora-rootfs: aarch64-fedora-34-rootfs
aarch64-fedora-rootfs: aarch64-fedora-35-rootfs
aarch64-fedora-rootfs: aarch64-fedora-36-rootfs

aarch64-fedora-33-rootfs: | aarch64-fedora-33-rootfs/etc/machine-id
aarch64-fedora-34-rootfs: | aarch64-fedora-34-rootfs/etc/machine-id
aarch64-fedora-35-rootfs: | aarch64-fedora-35-rootfs/etc/machine-id
aarch64-fedora-36-rootfs: | aarch64-fedora-36-rootfs/etc/machine-id

aarch64-fedora-33-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib64/ldb/modules/ldb/tdb.so:/usr/lib64/ldb/modules/ldb/mdb.so:/usr/lib64/ldb/modules/ldb/ldb.so
aarch64-fedora-33-rootfs/etc/machine-id:
aarch64-fedora-34-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib64/ldb/modules/ldb/tdb.so:/usr/lib64/ldb/modules/ldb/mdb.so:/usr/lib64/ldb/modules/ldb/ldb.so
aarch64-fedora-34-rootfs/etc/machine-id:

aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib64/ldb:/usr/lib64:/lib64
aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-aarch64-linux-aarch64/.*\.gcda|^$(CURDIR)/aarch64-fedora-$*-rootfs/var/log/dnf.rpm.log
aarch64-fedora-%-rootfs/etc/machine-id: | aarch64/libiamroot-linux-aarch64.so.1 x86_64/libiamroot-linux-x86-64.so.2
	install -D -m644 support/fedora.repo aarch64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --forcearch aarch64 --releasever $* --assumeyes --installroot $(CURDIR)/aarch64-fedora-$*-rootfs group install minimal-environment"
	rm -f aarch64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
endif

ifneq ($(shell command -v armv7l-unknown-linux-gnueabihf-gcc 2>/dev/null),)
armhfp-fedora-33-chroot:
armhfp-fedora-34-chroot:
armhfp-fedora-35-chroot:
armhfp-fedora-36-chroot:
armhfp-fedora-%-chroot: | armhfp-fedora-%-rootfs
	bash iamroot-shell -c "chroot armhfp-fedora-$*-rootfs"

arm-rootfs: armhfp-fedora-rootfs

.PHONY: armhfp-fedora-rootfs
armhfp-fedora-rootfs: armhfp-fedora-33-rootfs 
armhfp-fedora-rootfs: armhfp-fedora-34-rootfs 
armhfp-fedora-rootfs: armhfp-fedora-35-rootfs 
armhfp-fedora-rootfs: armhfp-fedora-36-rootfs

armhfp-fedora-33-rootfs: | armhfp-fedora-33-rootfs/etc/machine-id
armhfp-fedora-34-rootfs: | armhfp-fedora-34-rootfs/etc/machine-id
armhfp-fedora-35-rootfs: | armhfp-fedora-35-rootfs/etc/machine-id
armhfp-fedora-36-rootfs: | armhfp-fedora-36-rootfs/etc/machine-id

armhfp-fedora-33-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib/ldb/modules/ldb/tdb.so:/usr/lib/ldb/modules/ldb/mdb.so:/usr/lib/ldb/modules/ldb/ldb.so
armhfp-fedora-33-rootfs/etc/machine-id:
armhfp-fedora-34-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib/ldb/modules/ldb/tdb.so:/usr/lib/ldb/modules/ldb/mdb.so:/usr/lib/ldb/modules/ldb/ldb.so
armhfp-fedora-34-rootfs/etc/machine-id:

armhfp-fedora-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib/ldb:/usr/lib:/lib
armhfp-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_ALLOW = ^/var/run/(console|sepermit|faillock)
armhfp-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|var/run)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/output-armhf-linux-armhf/.*\.gcda|^$(CURDIR)/armhfp-fedora-$*-rootfs/var/log/dnf.rpm.log
armhfp-fedora-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_ARMHF_3 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2:/usr/lib/libpthread.so.0:/usr/lib/librt.so.1
armhfp-fedora-%-rootfs/etc/machine-id: | armhf/libiamroot-linux-armhf.so.3 x86_64/libiamroot-linux-x86-64.so.2
	install -D -m644 support/fedora.repo armhfp-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --forcearch armv7hl --releasever $* --assumeyes --installroot $(CURDIR)/armhfp-fedora-$*-rootfs group install minimal-environment"
	rm -f armhfp-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
endif
endif

ifneq ($(shell command -v aarch64-linux-musl-gcc 2>/dev/null),)
aarch64-rootfs: aarch64-alpine-rootfs

.PHONY: aarch64-mini-chroot
aarch64-mini-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-mini-rootfs
aarch64-mini-chroot: | aarch64/libiamroot-musl-aarch64.so.1 x86_64/libiamroot-linux-x86-64.so.2 aarch64-alpine-minirootfs
	bash iamroot-shell -c "chroot aarch64-alpine-minirootfs /bin/sh"

.PHONY: aarch64-alpine-minirootfs
aarch64-alpine-minirootfs: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-alpine-minirootfs
aarch64-alpine-minirootfs: | aarch64-alpine-minirootfs/bin/busybox

aarch64-alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.16.0-aarch64.tar.gz
	mkdir -p aarch64-alpine-minirootfs
	tar xf alpine-minirootfs-3.16.0-aarch64.tar.gz -C aarch64-alpine-minirootfs

alpine-minirootfs-3.13.0-aarch64.tar.gz:
alpine-minirootfs-%-aarch64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v$(basename $*)/releases/aarch64/alpine-minirootfs-$*-aarch64.tar.gz

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
aarch64-alpine-3.14-chroot:
aarch64-alpine-3.15-chroot:
aarch64-alpine-3.16-chroot:
aarch64-alpine-edge-chroot:
aarch64-alpine-%-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-alpine-$*-rootfs
aarch64-alpine-%-chroot: | aarch64-alpine-%-rootfs
	bash iamroot-shell -c "chroot aarch64-alpine-$*-rootfs"

.PHONY: aarch64-alpine-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.14-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.15-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.16-rootfs
aarch64-alpine-rootfs: aarch64-alpine-edge-rootfs

aarch64-alpine-3.14-rootfs: | aarch64-alpine-3.14-rootfs/bin/busybox
aarch64-alpine-3.15-rootfs: | aarch64-alpine-3.15-rootfs/bin/busybox
aarch64-alpine-3.16-rootfs: | aarch64-alpine-3.16-rootfs/bin/busybox
aarch64-alpine-edge-rootfs: | aarch64-alpine-edge-rootfs/bin/busybox

aarch64-alpine-%-rootfs/bin/busybox: | aarch64/libiamroot-musl-aarch64.so.1 x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "APK_OPTS='--arch aarch64' alpine-make-rootfs aarch64-alpine-$*-rootfs --keys-dir /usr/share/apk/keys/aarch64 --mirror-uri http://nl.alpinelinux.org/alpine --branch $*"
endif
endif

archlinux32.gpg archlinux32-trusted:
	wget https://git.archlinux32.org/archlinux32-keyring/plain/$@

archlinuxarm.gpg archlinuxarm-trusted:
	wget https://raw.githubusercontent.com/archlinuxarm/archlinuxarm-keyring/master/$@

manjaro.gpg manjaro-trusted:
	wget https://gitlab.manjaro.org/packages/core/manjaro-keyring/-/raw/master/$@

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
%.ext4: | x86_64/libiamroot-linux-x86-64.so.2 %-rootfs $(MODULESDIRS)
	$(MAKE) $*-postrootfs
	rm -f $@.tmp
	fallocate --length 2G $@.tmp
	bash iamroot-shell -c "mkfs.ext4 -d $*-rootfs $@.tmp"
	mv $@.tmp $@

.PRECIOUS: %-rootfs/usr/lib/modules/$(KVER) %-rootfs/usr/lib/modules/$(VMLINUX_KVER)
%-rootfs/usr/lib/modules/$(KVER) %-rootfs/usr/lib/modules/$(VMLINUX_KVER): | x86_64/libiamroot-linux-x86-64.so.2 %-rootfs
	rm -Rf $@.tmp $@
	mkdir -p $(@D)
	bash iamroot-shell -c "rsync -a /usr/lib/modules/$(@F)/. $@.tmp/."
	mv $@.tmp $@

.PHONY: %-postrootfs
%-postrootfs:

.PHONY: static-chroot
static-chroot: x86_64/libiamroot-linux-x86-64.so.2 | static-rootfs
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

.PHONY: support
support: arch-support
support: debian-support
support: ubuntu-support
support: fedora-support
support: alpine-support

.PHONY: extra-support
extra-support: manjaro-support opensuse-support

.PHONY: fixme-support
fixme-support:

.PHONY: arch-support
arch-support: support/arch-rootfs.txt
arch-support: support/i686-arch-rootfs.txt

.PHONY: manjaro-support
manjaro-support: support/manjaro-rootfs.txt

.PHONY: debian-support
debian-support: support/debian-oldoldstable-rootfs.txt
debian-support: support/debian-oldstable-rootfs.txt
debian-support: support/debian-stable-rootfs.txt
debian-support: support/debian-testing-rootfs.txt
debian-support: support/debian-unstable-rootfs.txt

.PHONY: ubuntu-support
ubuntu-support: support/ubuntu-bionic-rootfs.txt
ubuntu-support: support/ubuntu-focal-rootfs.txt
ubuntu-support: support/ubuntu-impish-rootfs.txt
ubuntu-support: support/ubuntu-jammy-rootfs.txt

.PHONY: fedora-support
fedora-support: support/fedora-33-rootfs.txt
fedora-support: support/fedora-34-rootfs.txt
fedora-support: support/fedora-35-rootfs.txt
fedora-support: support/fedora-36-rootfs.txt

.PHONY: opensuse-support
opensuse-support: support/opensuse-leaf-rootfs.txt
opensuse-support: support/opensuse-tumbleweed-rootfs.txt

.PHONY: alpine-support
alpine-support: support/alpine-3.14-rootfs.txt
alpine-support: support/alpine-3.15-rootfs.txt
# FIXME: Alpine Linux 3.16 index is currently broken.
# alpine-support: support/alpine-3.16-rootfs.txt
alpine-support: support/alpine-edge-rootfs.txt

.PRECIOUS: support/%arch-rootfs.txt
support/arch-rootfs.txt: arch-rootfs.log
	support/pacstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/i686-arch-rootfs.txt
support/i686-arch-rootfs.txt: i686-arch-rootfs.log
	support/pacstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/manjaro-rootfs.txt
support/manjaro-rootfs.txt: manjaro-rootfs.log
	support/pacstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/debian-oldoldstable-rootfs.txt
support/debian-oldoldstable-rootfs.txt: debian-oldoldstable-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/debian-oldstable-rootfs.txt
support/debian-oldstable-rootfs.txt: debian-oldstable-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/debian-stable-rootfs.txt
support/debian-stable-rootfs.txt: debian-stable-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/debian-testing-rootfs.txt
support/debian-testing-rootfs.txt: debian-testing-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/debian-unstable-rootfs.txt
support/debian-unstable-rootfs.txt: debian-unstable-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/ubuntu-bionic-rootfs.txt
support/ubuntu-bionic-rootfs.txt: ubuntu-bionic-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/ubuntu-focal-rootfs.txt
support/ubuntu-focal-rootfs.txt: ubuntu-focal-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/ubuntu-impish-rootfs.txt
support/ubuntu-impish-rootfs.txt: ubuntu-impish-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/ubuntu-jammy-rootfs.txt
support/ubuntu-jammy-rootfs.txt: ubuntu-jammy-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/fedora-33-rootfs.txt
support/fedora-33-rootfs.txt: fedora-33-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/fedora-34-rootfs.txt
support/fedora-34-rootfs.txt: fedora-34-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/fedora-35-rootfs.txt
support/fedora-35-rootfs.txt: fedora-35-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/fedora-36-rootfs.txt
support/fedora-36-rootfs.txt: fedora-36-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/opensuse-leaf-rootfs.txt
support/opensuse-leaf-rootfs.txt: opensuse-leaf-rootfs.log
	support/zypper.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/opensuse-tumbleweed-rootfs.txt
support/opensuse-tumbleweed-rootfs.txt: opensuse-tumbleweed-rootfs.log
	support/zypper.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/alpine-3.14-rootfs.txt
support/alpine-3.14-rootfs.txt: alpine-3.14-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/alpine-3.15-rootfs.txt
support/alpine-3.15-rootfs.txt: alpine-3.15-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/alpine-3.16-rootfs.txt
support/alpine-3.16-rootfs.txt: alpine-3.16-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/alpine-edge-rootfs.txt
support/alpine-edge-rootfs.txt: alpine-edge-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PHONY: log
log: arch-log
log: debian-log
log: ubuntu-log
log: fedora-log
log: alpine-log

.PHONY: extra-log
extra-log: manjaro-log opensuse-log

.PHONY: fixme-log
fixme-log:

.PHONY: arch-log
arch-log: arch-rootfs.log
arch-log: i686-arch-rootfs.log

.PHONY: manjaro-log
manjaro-log: manjaro-rootfs.log

.PHONY: debian-log
debian-log: debian-oldoldstable-rootfs.log
debian-log: debian-oldstable-rootfs.log
debian-log: debian-stable-rootfs.log
debian-log: debian-testing-rootfs.log
debian-log: debian-unstable-rootfs.log

.PHONY: ubuntu-log
ubuntu-log: ubuntu-bionic-rootfs.log
ubuntu-log: ubuntu-focal-rootfs.log
ubuntu-log: ubuntu-impish-rootfs.log
ubuntu-log: ubuntu-jammy-rootfs.log

.PHONY: fedora-log
fedora-log: fedora-33-rootfs.log
fedora-log: fedora-34-rootfs.log
fedora-log: fedora-35-rootfs.log
fedora-log: fedora-36-rootfs.log

.PHONY: opensuse-log
opensuse-log: opensuse-leaf-rootfs.log
opensuse-log: opensuse-tumbleweed-rootfs.log

.PHONY: alpine-log
alpine-log: alpine-3.14-rootfs.log
alpine-log: alpine-3.15-rootfs.log
alpine-log: alpine-3.16-rootfs.log
alpine-log: alpine-edge-rootfs.log

.PHONY: %.log
.PRECIOUS: %.log
arch-rootfs.log:
i686-arch-rootfs.log:
debian-oldoldstable-rootfs.log:
debian-oldstable-rootfs.log:
debian-stable-rootfs.log:
debian-testing-rootfs.log:
debian-unstable-rootfs.log:
ubuntu-bionic-rootfs.log:
ubuntu-focal-rootfs.log:
ubuntu-impish-rootfs.log:
ubuntu-jammy-rootfs.log:
fedora-33-rootfs.log:
fedora-34-rootfs.log:
fedora-35-rootfs.log:
fedora-36-rootfs.log:
opensuse-leaf-rootfs-log:
opensuse-tumbleweed-rootfs-log:
alpine-3.14-rootfs.log:
alpine-3.15-rootfs.log:
alpine-3.16-rootfs.log:
alpine-edge-rootfs.log:
%.log: SHELL = /bin/bash -o pipefail
%.log:
	$(MAKE) --silent $* 2>&1 | tee $@.tmp
	mv $@.tmp $@

%-debug %-debug1:
	$(MAKE) $* IAMROOT_DEBUG=1

%-debug2:
	$(MAKE) $* IAMROOT_DEBUG=2

%-debug3:
	$(MAKE) $* IAMROOT_DEBUG=3

%-debug4:
	$(MAKE) $* IAMROOT_DEBUG=4

%-debug5:
	$(MAKE) $* IAMROOT_DEBUG=5

%-debug6:
	$(MAKE) $* IAMROOT_DEBUG=6

%:
	$(MAKE) -f Makefile $@
