#
# Copyright 2021-2023 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

PREFIX ?= /usr/local
ARCH ?= $(shell uname -m 2>/dev/null)
KVER ?= $(shell uname -r 2>/dev/null)
VMLINUX_KVER ?= $(shell vmlinux --version 2>/dev/null)

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

IAMROOT_LIB_MUSL_I386_1 = $(CURDIR)/i686/libiamroot-musl-i386.so.1
export IAMROOT_LIB_MUSL_I386_1

IAMROOT_LIB_MUSL_X86_64_1 = $(CURDIR)/x86_64/libiamroot-musl-x86_64.so.1
export IAMROOT_LIB_MUSL_X86_64_1

IAMROOT_LIB_MUSL_ARMHF_1 = $(CURDIR)/armhf/libiamroot-musl-armhf.so.1
export IAMROOT_LIB_MUSL_ARMHF_1

IAMROOT_LIB_MUSL_AARCH64_1 = $(CURDIR)/aarch64/libiamroot-musl-aarch64.so.1
export IAMROOT_LIB_MUSL_AARCH64_1

IAMROOT_EXEC = $(CURDIR)/exec.sh
export IAMROOT_EXEC

IAMROOT_EXEC_IGNORE = ldd|mountpoint
export IAMROOT_EXEC_IGNORE

ifeq ($(ARCH),x86_64)
IAMROOT_LIB = $(IAMROOT_LIB_LINUX_X86_64_2)
export IAMROOT_LIB
endif

ifeq ($(ARCH),aarch64)
IAMROOT_LIB = $(IAMROOT_LIB_LINUX_AARCH64_1)
export IAMROOT_LIB
endif

-include local.mk

MAKEFLAGS += --no-print-directory

.PHONY: all
all:

define libiamroot_so =
all: $(1)/libiamroot-$(2).so.$(3)
ci: $(1)/libiamroot-$(2).so.$(3)

.PRECIOUS: $(1)/libiamroot-$(2).so.$(3)
$(1)/libiamroot-$(2).so.$(3): output-$(1)-$(2)/libiamroot.so
	install -D -m755 $$< $$@

output-$(1)-$(2)/libiamroot.so:

install: install-exec-$(1)-$(2).$(3)

.PHONY: install-exec-$(1)-$(2).$(3)
install-exec-$(1)-$(2).$(3):
	install -D -m755 $(1)/libiamroot-$(2).so.$(3) $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so.$(3)
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

define run
.PHONY: qemu-system-$(1)-$(2)
qemu-system-$(1)-$(2): override CMDLINE += panic=5
qemu-system-$(1)-$(2): override CMDLINE += console=ttyS0
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -enable-kvm -m 4G -machine q35 -smp 4 -cpu host
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -nographic -serial mon:stdio
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -kernel /boot/vmlinuz-linux
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -initrd initrd-rootfs.cpio
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -drive file=$(1)-$(2).ext4,if=virtio
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -append "$$(CMDLINE)"
qemu-system-$(1)-$(2): | $(1)-$(2).ext4 initrd-rootfs.cpio
	qemu-system-x86_64 $$(QEMUSYSTEMFLAGS)

ifneq ($(VMLINUX_KVER),)
.PHONY: vmlinux-$(2)
vmlinux-$(2): override VMLINUXFLAGS+=panic=5
vmlinux-$(2): override VMLINUXFLAGS+=console=tty0 con0=fd:0,fd:1 con=none
vmlinux-$(2): override VMLINUXFLAGS+=mem=256M
vmlinux-$(2): override VMLINUXFLAGS+=rw
vmlinux-$(2): override VMLINUXFLAGS+=ubd0=$(1)-$(2).ext4
vmlinux-$(2): override VMLINUXFLAGS+=$$(CMDLINE)
vmlinux-$(2): | $(1)-$(2).ext4
	settings=$$$$(stty -g); \
	if ! vmlinux $$(VMLINUXFLAGS); then \
		echo stty "$$$$settings"; \
		false; \
	fi; \
	stty "$$$$settings"
endif

.PRECIOUS: $(1)-$(2).ext4
ifneq ($(KVER),)
MODULESDIRS_$(1)-$(2) += $(1)-$(2)-rootfs/usr/lib/modules/$(KVER)
endif
ifneq ($(VMLINUX_KVER),)
MODULESDIRS_$(1)-$(2) += $(1)-$(2)-rootfs/usr/lib/modules/$(VMLINUX_KVER)
endif
$(1)-$(2).ext4: | x86_64/libiamroot-linux-x86-64.so.2 $(1)-$(2)-rootfs $$(MODULESDIRS_$(1)-$(2))
	$(MAKE) $(1)-$(2)-postrootfs
	rm -f $$@.tmp
	fallocate --length 2G $$@.tmp
	bash iamroot-shell -c "mkfs.ext4 -d $(1)-$(2)-rootfs $$@.tmp"
	mv $$@.tmp $$@

.PHONY: $(1)-$(2)-postrootfs
$(1)-$(2)-postrootfs:

.PRECIOUS: $(1)-$(2)-rootfs/usr/lib/modules/$(KVER) $(1)-$(2)-rootfs/usr/lib/modules/$(VMLINUX_KVER)
$(1)-$(2)-rootfs/usr/lib/modules/$(KVER) $(1)-$(2)-rootfs/usr/lib/modules/$(VMLINUX_KVER): | x86_64/libiamroot-linux-x86-64.so.2 $(1)-$(2)-rootfs
	rm -Rf $$@.tmp $$@
	mkdir -p $$(@D)
	bash iamroot-shell -c "rsync -a /usr/lib/modules/$$(@F)/. $$@.tmp/."
	mv $$@.tmp $$@

.PHONY: chroot-$(1)-$(2)
chroot-$(1)-$(2):
	$(MAKE) mount-$(1)-$(2)
	-sudo chroot mnt $(SHELL)
	$(MAKE) umount-$(1)-$(2)

.PHONY: mount-$(1)-$(2)
mount-$(1)-$(2): | $(1)-$(2).ext4 mnt
	sudo mount -oloop $(1)-$(2).ext4 mnt

.PHONY: umount-$(1)-$(2)
umount-$(1)-$(2): | mnt
	sudo umount mnt
endef

define pacstrap-postrootfs
$(1)-$(2)-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	mkdir -p $(1)-$(2)-rootfs/var/lib/systemd/linger
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs systemctl enable getty@tty0.service"
endef

define debootstrap-postrootfs
$(1)-$(2)-postrootfs: export IAMROOT_LIBRARY_PATH = /lib/x86_64-linux-gnu:/lib:/usr/lib/x86_64-linux-gnu:/usr/lib
$(1)-$(2)-postrootfs: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2
$(1)-$(2)-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs systemctl enable getty@tty0.service"
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/multi-user.target.wants/sshd.service
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs systemctl disable sshd.service"
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs pam-auth-update"
endef

define dnf-postrootfs
$(1)-$(2)-postrootfs: export IAMROOT_LIBRARY_PATH = /lib64:/usr/lib64
$(1)-$(2)-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	touch $(1)-$(2)-rootfs/etc/systemd/zram-generator.conf
	mkdir -p $(1)-$(2)-rootfs/var/lib/systemd/linger
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs systemctl enable getty@tty0.service"
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/multi-user.target.wants/sshd.service
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs systemctl disable sshd.service"
endef

define zypper-postrootfs
$(1)-$(2)-postrootfs: export IAMROOT_LIBRARY_PATH = /lib64:/usr/lib64
$(1)-$(2)-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root:\*:/s,^root:\*:,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs pam-config -a --nullok"
	mkdir -p $(1)-$(2)-rootfs/var/lib/systemd/linger
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs systemctl enable getty@tty0.service"
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@ttyS0.service
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs systemctl enable getty@ttyS0.service"
endef

define alpine-postrootfs
$(1)-$(2)-postrootfs:
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^UNKNOWN$$:/d' \
	    -e '1iUNKNOWN' \
	    -i $(1)-$(2)-rootfs/etc/securetty
	sed -e '/^tty1:/itty0::respawn:/sbin/getty 38400 tty0' \
	    -e '/^tty[1-9]:/s,^,#,' \
	    -e '/^#ttyS0:/s,^#,,g' \
	    -i $(1)-$(2)-rootfs/etc/inittab
	chmod +r $(1)-$(2)-rootfs/bin/bbsuid
endef

export CC
export CFLAGS

ifeq ($(ARCH),x86_64)
libiamroot.so: x86_64/libiamroot-linux-x86-64.so.2
	install -D -m755 $< $@

$(eval $(call libiamroot_so,x86_64,linux-x86-64,2))
test: x86_64/libiamroot-linux-x86-64.so.2

output-i686-linux/libiamroot.so: override CC += -m32
output-i686-linux/libiamroot.so: override CFLAGS += -fno-stack-protector
$(eval $(call libiamroot_so,i686,linux,2))

ifneq ($(shell command -v arm-linux-gnueabi-gcc 2>/dev/null),)
output-arm-linux/libiamroot.so: CC = arm-linux-gnueabi-gcc
$(eval $(call libiamroot_so,arm,linux,3))
endif

ifneq ($(shell command -v arm-linux-gnueabihf-gcc 2>/dev/null),)
output-armhf-linux-armhf/libiamroot.so: CC = arm-linux-gnueabihf-gcc
$(eval $(call libiamroot_so,armhf,linux-armhf,3))
endif

ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
output-aarch64-linux-aarch64/libiamroot.so: CC = aarch64-linux-gnu-gcc
$(eval $(call libiamroot_so,aarch64,linux-aarch64,1))
endif

ifneq ($(shell command -v i386-musl-gcc 2>/dev/null),)
output-i686-musl-i386/libiamroot.so: CC = i386-musl-gcc
output-i686-musl-i386/libiamroot.so: override CFLAGS += -fno-stack-protector
$(eval $(call libiamroot_so,i686,musl-i386,1))
endif

ifneq ($(shell command -v musl-gcc 2>/dev/null),)
output-x86_64-musl-x86_64/libiamroot.so: CC = musl-gcc
$(eval $(call libiamroot_so,x86_64,musl-x86_64,1))
endif

ifneq ($(shell command -v arm-linux-musleabihf-gcc 2>/dev/null),)
output-armhf-musl-armhf/libiamroot.so: CC = arm-linux-musleabihf-gcc
$(eval $(call libiamroot_so,armhf,musl-armhf,1))
endif

ifneq ($(shell command -v aarch64-linux-musl-gcc 2>/dev/null),)
output-aarch64-musl-aarch64/libiamroot.so: CC = aarch64-linux-musl-gcc
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
endif

ifeq ($(ARCH),aarch64)
libiamroot.so: aarch64/libiamroot-linux-aarch64.so.1
	install -D -m755 $< $@

$(eval $(call libiamroot_so,aarch64,linux-aarch64,1))
test: aarch64/libiamroot-linux-aarch64.so.1
endif

.PRECIOUS: output-%/libiamroot.so
output-%/libiamroot.so: $(wildcard *.c) | output-%
	$(MAKE) -f $(CURDIR)/Makefile -C output-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: output-%
output-%:
	mkdir $@

.PHONY: fixme-rootfs
fixme-rootfs:

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
test:
	$(MAKE) -f Makefile $@

.PHONY: coverage
coverage: gcov/index.html

.PHONY: gcov/index.html
gcov/index.html:
	mkdir -p $(@D)
	gcovr --html-details --html-title iamroot -e "tests/.*\.c" -s -o $@ output-x86_64-linux-x86-64/ tests/

.PHONY: cobertura.xml
cobertura.xml:
	gcovr --cobertura -e "tests/.*\.c" -s -o $@ output-x86_64-linux-x86-64/ tests/

.PHONY: codacy
codacy: cobertura.xml
	bash <(curl -Ls https://coverage.codacy.com/get.sh)

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -f cobertura.xml
	rm -f *.gcda *.gcno
	rm -f *.ext4 *.cpio
	rm -f *-rootfs.log
	rm -Rf *-rootfs/
	rm -Rf *-minirootfs/

ifeq ($(ARCH),x86_64)
ifneq ($(shell command -v pacstrap 2>/dev/null),)
.PHONY: arch-test
arch-test: | x86_64-arch-rootfs/usr/bin/shebang.sh
arch-test: | x86_64-arch-rootfs/usr/bin/shebang-arg.sh
arch-test: $(subst $(CURDIR)/,,$(IAMROOT_LIB)) | x86_64-arch-rootfs
	bash iamroot-shell -c "chroot x86_64-arch-rootfs shebang.sh one two three"
	bash iamroot-shell -c "chroot x86_64-arch-rootfs shebang-arg.sh one two three"

x86_64-arch-rootfs/usr/bin/%: support/% | x86_64-arch-rootfs
	cp $< $@

.PHONY: x86_64-arch-chroot
x86_64-arch-chroot: | x86_64-arch-rootfs
	bash iamroot-shell -c "chroot x86_64-arch-rootfs"

rootfs: x86_64-arch-rootfs

.PHONY: x86_64-arch-rootfs
x86_64-arch-rootfs: | x86_64-arch-rootfs/etc/machine-id

x86_64-arch-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
x86_64-arch-rootfs/etc/machine-id: export EUID = 0
x86_64-arch-rootfs/etc/machine-id: $(subst $(CURDIR)/,,$(IAMROOT_LIB))
	mkdir x86_64-arch-rootfs
	bash iamroot-shell -c "pacstrap -GMC support/x86_64-arch-pacman.conf x86_64-arch-rootfs"

i686-rootfs: i686-arch-rootfs

.PHONY: i686-arch-chroot
i686-arch-chroot: export QEMU_LD_PREFIX = $(CURDIR)/i686-arch-rootfs
i686-arch-chroot: export IAMROOT_LD_PRELOAD_LINUX_2 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2
i686-arch-chroot: | i686-arch-rootfs
	bash iamroot-shell -c "chroot i686-arch-rootfs"

.PHONY: i686-arch-rootfs
i686-arch-rootfs: export QEMU_LD_PREFIX = $(CURDIR)/i686-arch-rootfs
i686-arch-rootfs: | i686-arch-rootfs/etc/machine-id

i686-arch-rootfs/etc/machine-id: export QEMU_LD_PREFIX = $(CURDIR)/i686-arch-rootfs
i686-arch-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
i686-arch-rootfs/etc/machine-id: export EUID = 0
i686-arch-rootfs/etc/machine-id: | i686/libiamroot-linux.so.2 x86_64/libiamroot-linux-x86-64.so.2
	mkdir i686-arch-rootfs
	bash iamroot-shell -c "pacstrap -GMC support/i686-arch-pacman.conf i686-arch-rootfs"

$(eval $(call run,x86_64,arch))

$(eval $(call pacstrap-postrootfs,x86_64,arch))

x86_64-manjaro-stable-chroot:
x86_64-manjaro-%-chroot: | x86_64-manjaro-%-rootfs
	bash iamroot-shell -c "chroot x86_64-manjaro-$*-rootfs"

extra-rootfs: x86_64-manjaro-stable-rootfs

.PHONY: x86_64-manjaro-stable-rootfs
x86_64-manjaro-stable-rootfs: | x86_64-manjaro-stable-rootfs/etc/machine-id

x86_64-manjaro-stable-rootfs/etc/machine-id:
x86_64-manjaro-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
x86_64-manjaro-%-rootfs/etc/machine-id: export EUID = 0
x86_64-manjaro-%-rootfs/etc/machine-id: | x86_64/libiamroot-linux-x86-64.so.2
	mkdir x86_64-manjaro-$*-rootfs
	bash iamroot-shell -c "pacstrap -GMC support/x86_64-manjaro-$*-pacman.conf x86_64-manjaro-$*-rootfs base"

$(eval $(call run,x86_64,manjaro-stable))

$(eval $(call pacstrap-postrootfs,x86_64,manjaro-stable))
endif

ifneq ($(shell command -v debootstrap 2>/dev/null),)
x86_64-debian-oldoldstable-chroot:
x86_64-debian-oldstable-chroot:
x86_64-debian-stable-chroot:
x86_64-debian-testing-chroot:
x86_64-debian-unstable-chroot:
x86_64-debian-%-chroot: export IAMROOT_LIBRARY_PATH = /lib/x86_64-linux-gnu:/lib:/usr/lib/x86_64-linux-gnu:/usr/lib
x86_64-debian-%-chroot: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2:/lib/x86_64-linux-gnu/libpthread.so.0
x86_64-debian-%-chroot: export IAMROOT_LD_PRELOAD_LINUX_AARCH64_1 = /lib/aarch64-linux-gnu/libc.so.6:/lib/aarch64-linux-gnu/libdl.so.2:/lib/aarch64-linux-gnu/libpthread.so.0
x86_64-debian-%-chroot: | x86_64-debian-%-rootfs
	bash iamroot-shell -c "chroot x86_64-debian-$*-rootfs"

rootfs: x86_64-debian-rootfs

.PHONY: x86_64-debian-rootfs
x86_64-debian-rootfs: x86_64-debian-oldoldstable-rootfs
x86_64-debian-rootfs: x86_64-debian-oldstable-rootfs
x86_64-debian-rootfs: x86_64-debian-stable-rootfs
x86_64-debian-rootfs: x86_64-debian-testing-rootfs
x86_64-debian-rootfs: x86_64-debian-unstable-rootfs

x86_64-debian-oldoldstable-rootfs: | x86_64-debian-oldoldstable-rootfs/etc/machine-id
x86_64-debian-oldstable-rootfs: | x86_64-debian-oldstable-rootfs/etc/machine-id
x86_64-debian-stable-rootfs: | x86_64-debian-stable-rootfs/etc/machine-id
x86_64-debian-testing-rootfs: | x86_64-debian-testing-rootfs/etc/machine-id
x86_64-debian-unstable-rootfs: | x86_64-debian-unstable-rootfs/etc/machine-id

x86_64-debian-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /lib/x86_64-linux-gnu:/lib:/usr/lib/x86_64-linux-gnu:/usr/lib
x86_64-debian-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2:/lib/x86_64-linux-gnu/libpthread.so.0
x86_64-debian-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_AARCH64_1 = /lib/aarch64-linux-gnu/libc.so.6:/lib/aarch64-linux-gnu/libdl.so.2:/lib/aarch64-linux-gnu/libpthread.so.0
# chfn: PAM: Critical error - immediate abort
# adduser: `/usr/bin/chfn -f systemd Network Management systemd-network' returned error code 1. Exiting.
x86_64-debian-%-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|chfn
x86_64-debian-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda
# System has not been booted with systemd as init system (PID 1). Can't operate.
# Failed to connect to bus: Host is down
# invoke-rc.d: could not determine current runlevel
x86_64-debian-%-rootfs/etc/machine-id: export SYSTEMD_OFFLINE = 1
# debconf: PERL_DL_NONLAZY is not set, if debconf is running from a preinst script, this is not safe
x86_64-debian-%-rootfs/etc/machine-id: export PERL_DL_NONLAZY = 1
x86_64-debian-%-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://deb.debian.org/debian
x86_64-debian-%-rootfs/etc/machine-id: export DEBOOTSTRAPFLAGS ?= --merged-usr --no-check-gpg
x86_64-debian-%-rootfs/etc/machine-id: | $(subst $(CURDIR)/,,$(IAMROOT_LIB))
	mkdir -p x86_64-debian-$*-rootfs
	bash iamroot-shell -c "debootstrap --keep-debootstrap-dir $(DEBOOTSTRAPFLAGS) $* x86_64-debian-$*-rootfs $(DEBOOTSTRAP_MIRROR)"
	cat x86_64-debian-$*-rootfs/debootstrap/debootstrap.log
	rm -Rf x86_64-debian-$*-rootfs/debootstrap/

$(eval $(call run,x86_64,debian-oldoldstable))
$(eval $(call run,x86_64,debian-oldstable))
$(eval $(call run,x86_64,debian-stable))
$(eval $(call run,x86_64,debian-testing))
$(eval $(call run,x86_64,debian-unstable))

qemu-system-x86_64-debian-oldoldstable: override CMDLINE += rw
qemu-system-x86_64-debian-oldstable: override CMDLINE += rw
qemu-system-x86_64-debian-stable: override CMDLINE += rw
qemu-system-x86_64-debian-testing: override CMDLINE += rw
qemu-system-x86_64-debian-unstable: override CMDLINE += rw

x86_64-debian-oldoldstable-postrootfs: export IAMROOT_LIBRARY_PATH = /lib/x86_64-linux-gnu:/lib:/usr/lib/x86_64-linux-gnu:/usr/lib
x86_64-debian-oldoldstable-postrootfs: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2
x86_64-debian-oldoldstable-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i x86_64-debian-oldoldstable-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i x86_64-debian-oldoldstable-rootfs/etc/shadow
	rm -f x86_64-debian-oldoldstable-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell -c "chroot x86_64-debian-oldoldstable-rootfs systemctl enable getty@tty0.service"
	bash iamroot-shell -c "chroot x86_64-debian-oldoldstable-rootfs pam-auth-update"

$(eval $(call debootstrap-postrootfs,x86_64,debian-oldstable))
$(eval $(call debootstrap-postrootfs,x86_64,debian-stable))
$(eval $(call debootstrap-postrootfs,x86_64,debian-testing))
$(eval $(call debootstrap-postrootfs,x86_64,debian-unstable))

x86_64-ubuntu-trusty-chroot:
x86_64-ubuntu-xenial-chroot:
x86_64-ubuntu-bionic-chroot:
x86_64-ubuntu-focal-chroot:
x86_64-ubuntu-jammy-chroot:
x86_64-ubuntu-kinetic-chroot:
x86_64-ubuntu-%-chroot: export IAMROOT_LIBRARY_PATH = /lib/x86_64-linux-gnu:/lib:/usr/lib/x86_64-linux-gnu:/usr/lib
x86_64-ubuntu-%-chroot: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2:/lib/x86_64-linux-gnu/libpthread.so.0
x86_64-ubuntu-%-chroot: export IAMROOT_LD_PRELOAD_LINUX_AARCH64_1 = /lib/aarch64-linux-gnu/libc.so.6:/lib/aarch64-linux-gnu/libdl.so.2:/lib/aarch64-linux-gnu/libpthread.so.0
x86_64-ubuntu-%-chroot: | x86_64-ubuntu-%-rootfs
	bash iamroot-shell -c "chroot x86_64-ubuntu-$*-rootfs"

.PHONY: x86_64-ubuntu-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-trusty-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-xenial-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-bionic-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-focal-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-jammy-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-kinetic-rootfs

x86_64-ubuntu-trusty-rootfs: | x86_64-ubuntu-trusty-rootfs/etc/machine-id
x86_64-ubuntu-xenial-rootfs: | x86_64-ubuntu-xenial-rootfs/etc/machine-id
x86_64-ubuntu-bionic-rootfs: | x86_64-ubuntu-bionic-rootfs/etc/machine-id
x86_64-ubuntu-focal-rootfs: | x86_64-ubuntu-focal-rootfs/etc/machine-id
x86_64-ubuntu-jammy-rootfs: | x86_64-ubuntu-jammy-rootfs/etc/machine-id
x86_64-ubuntu-kinetic-rootfs: | x86_64-ubuntu-kinetic-rootfs/etc/machine-id

x86_64-ubuntu-trusty-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
x86_64-ubuntu-trusty-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|/var/lib/dpkg/info/(initscripts|initramfs-tools).postinst
x86_64-ubuntu-trusty-rootfs/etc/machine-id:

x86_64-ubuntu-xenial-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
# chfn: PAM: Critical error - immediate abort
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
x86_64-ubuntu-xenial-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|chfn|/var/lib/dpkg/info/initramfs-tools.postinst
x86_64-ubuntu-xenial-rootfs/etc/machine-id:

# chfn: PAM: Critical error - immediate abort
# adduser: `/usr/bin/chfn -f systemd Network Management systemd-network' returned error code 1. Exiting.
x86_64-ubuntu-bionic-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|chfn|/var/lib/dpkg/info/initramfs-tools.postinst
x86_64-ubuntu-bionic-rootfs/etc/machine-id:

x86_64-ubuntu-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /lib/x86_64-linux-gnu:/lib:/usr/lib/x86_64-linux-gnu:/usr/lib
x86_64-ubuntu-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libdl.so.2:/lib/x86_64-linux-gnu/libpthread.so.0
x86_64-ubuntu-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_AARCH64_1 = /lib/aarch64-linux-gnu/libc.so.6:/lib/aarch64-linux-gnu/libdl.so.2:/lib/aarch64-linux-gnu/libpthread.so.0
# chfn: PAM: Critical error - immediate abort
# adduser: `/usr/bin/chfn -f systemd Network Management systemd-network' returned error code 1. Exiting.
x86_64-ubuntu-%-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|chfn
x86_64-ubuntu-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda
# Processing triggers for libc-bin ...
# dpkg: cycle found while processing triggers:
#  chain of packages whose triggers are or may be responsible:
#   libc-bin -> libc-bin
#  packages' pending triggers which are or may be unresolvable:
#   libc-bin: ldconfig
# dpkg: error processing package libc-bin (--configure):
#  triggers looping, abandoned
# Errors were encountered while processing:
#  libc-bin
x86_64-ubuntu-%-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
# System has not been booted with systemd as init system (PID 1). Can't operate.
# Failed to connect to bus: Host is down
# invoke-rc.d: could not determine current runlevel
x86_64-ubuntu-%-rootfs/etc/machine-id: export SYSTEMD_OFFLINE = 1
# debconf: PERL_DL_NONLAZY is not set, if debconf is running from a preinst script, this is not safe
x86_64-ubuntu-%-rootfs/etc/machine-id: export PERL_DL_NONLAZY = 1
x86_64-ubuntu-%-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
x86_64-ubuntu-%-rootfs/etc/machine-id: export DEBOOTSTRAPFLAGS ?= --merged-usr --no-check-gpg
x86_64-ubuntu-%-rootfs/etc/machine-id: | $(subst $(CURDIR)/,,$(IAMROOT_LIB))
	mkdir -p x86_64-ubuntu-$*-rootfs
	bash iamroot-shell -c "debootstrap --keep-debootstrap-dir $(DEBOOTSTRAPFLAGS) $* x86_64-ubuntu-$*-rootfs $(DEBOOTSTRAP_MIRROR)"
	cat x86_64-ubuntu-$*-rootfs/debootstrap/debootstrap.log
	rm -Rf x86_64-ubuntu-$*-rootfs/debootstrap/

$(eval $(call run,x86_64,ubuntu-trusty))
$(eval $(call run,x86_64,ubuntu-xenial))
$(eval $(call run,x86_64,ubuntu-bionic))
$(eval $(call run,x86_64,ubuntu-focal))
$(eval $(call run,x86_64,ubuntu-jammy))
$(eval $(call run,x86_64,ubuntu-kinetic))

qemu-system-x86_64-ubuntu-trusty: override CMDLINE += rw
qemu-system-x86_64-ubuntu-xenial: override CMDLINE += rw
qemu-system-x86_64-ubuntu-bionic: override CMDLINE += rw
qemu-system-x86_64-ubuntu-focal: override CMDLINE += rw
qemu-system-x86_64-ubuntu-jammy: override CMDLINE += rw
qemu-system-x86_64-ubuntu-kinetic: override CMDLINE += rw

$(eval $(call debootstrap-postrootfs,x86_64,ubuntu-trusty))
$(eval $(call debootstrap-postrootfs,x86_64,ubuntu-xenial))
$(eval $(call debootstrap-postrootfs,x86_64,ubuntu-bionic))
$(eval $(call debootstrap-postrootfs,x86_64,ubuntu-focal))
$(eval $(call debootstrap-postrootfs,x86_64,ubuntu-jammy))
$(eval $(call debootstrap-postrootfs,x86_64,ubuntu-kinetic))
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
x86_64-fedora-33-chroot:
x86_64-fedora-34-chroot:
x86_64-fedora-35-chroot:
x86_64-fedora-36-chroot:
x86_64-fedora-37-chroot:
x86_64-fedora-%-chroot: export IAMROOT_LIBRARY_PATH = /lib64:/usr/lib64
x86_64-fedora-%-chroot: | x86_64-fedora-%-rootfs
	bash iamroot-shell -c "chroot x86_64-fedora-$*-rootfs"

rootfs: x86_64-fedora-rootfs

.PHONY: x86_64-fedora-rootfs
fedora-rootfs: x86_64-fedora-33-rootfs
fedora-rootfs: x86_64-fedora-34-rootfs
fedora-rootfs: x86_64-fedora-35-rootfs
fedora-rootfs: x86_64-fedora-36-rootfs
fedora-rootfs: x86_64-fedora-37-rootfs

x86_64-fedora-33-rootfs: | x86_64-fedora-33-rootfs/etc/machine-id
x86_64-fedora-34-rootfs: | x86_64-fedora-34-rootfs/etc/machine-id
x86_64-fedora-35-rootfs: | x86_64-fedora-35-rootfs/etc/machine-id
x86_64-fedora-36-rootfs: | x86_64-fedora-36-rootfs/etc/machine-id
x86_64-fedora-37-rootfs: | x86_64-fedora-37-rootfs/etc/machine-id

x86_64-fedora-33-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib64/ldb/modules/ldb/tdb.so:/usr/lib64/ldb/modules/ldb/mdb.so:/usr/lib64/ldb/modules/ldb/ldb.so
x86_64-fedora-33-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-33-rootfs/etc/machine-id:

x86_64-fedora-34-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib64/ldb/modules/ldb/tdb.so:/usr/lib64/ldb/modules/ldb/mdb.so:/usr/lib64/ldb/modules/ldb/ldb.so
x86_64-fedora-34-rootfs/etc/machine-id:

x86_64-fedora-35-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-35-rootfs/etc/machine-id:

x86_64-fedora-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib64/ldb:/lib64:/usr/lib64
x86_64-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/x86_64-fedora-$*-rootfs/var/log/dnf.rpm.log
x86_64-fedora-%-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora.repo
x86_64-fedora-%-rootfs/etc/machine-id: | x86_64/libiamroot-linux-x86-64.so.2
	install -D -m644 $(FEDORA_REPO) x86_64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --releasever $* --assumeyes --installroot $(CURDIR)/x86_64-fedora-$*-rootfs group install minimal-environment"
	rm -f x86_64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo

$(eval $(call run,x86_64,fedora-33))
$(eval $(call run,x86_64,fedora-34))
$(eval $(call run,x86_64,fedora-35))
$(eval $(call run,x86_64,fedora-36))
$(eval $(call run,x86_64,fedora-37))

qemu-system-x86_64-fedora-33: override CMDLINE += rw
qemu-system-x86_64-fedora-34: override CMDLINE += rw
qemu-system-x86_64-fedora-35: override CMDLINE += rw
qemu-system-x86_64-fedora-36: override CMDLINE += rw
qemu-system-x86_64-fedora-37: override CMDLINE += rw

$(eval $(call dnf-postrootfs,x86_64,fedora-33))
$(eval $(call dnf-postrootfs,x86_64,fedora-34))
$(eval $(call dnf-postrootfs,x86_64,fedora-35))
$(eval $(call dnf-postrootfs,x86_64,fedora-36))
$(eval $(call dnf-postrootfs,x86_64,fedora-37))
endif

ifneq ($(shell command -v zypper 2>/dev/null),)
x86_64-opensuse-leap-chroot: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib64/libc.so.6:/lib64/libdl.so.2
x86_64-opensuse-leap-chroot:
x86_64-opensuse-tumbleweed-chroot:
x86_64-opensuse-%-chroot: export IAMROOT_LIBRARY_PATH = /lib64:/usr/lib64
x86_64-opensuse-%-chroot: | x86_64-opensuse-%-rootfs
	bash iamroot-shell -c "chroot x86_64-opensuse-$*-rootfs"

extra-rootfs: opensuse-rootfs

.PHONY: opensuse-rootfs
opensuse-rootfs: | x86_64-opensuse-tumbleweed-rootfs

x86_64-opensuse-leap-rootfs: | x86_64-opensuse-leap-rootfs/etc/machine-id
x86_64-opensuse-tumbleweed-rootfs: | x86_64-opensuse-tumbleweed-rootfs/etc/machine-id

x86_64-opensuse-leap-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib64/libc.so.6:/lib64/libdl.so.2
x86_64-opensuse-leap-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|/usr/bin/chkstat|/usr/sbin/update-ca-certificates
x86_64-opensuse-leap-rootfs/etc/machine-id:

x86_64-opensuse-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /lib64:/usr/lib64
x86_64-opensuse-%-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|/usr/bin/chkstat
x86_64-opensuse-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda
x86_64-opensuse-%-rootfs/etc/machine-id: | x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "zypper --root $(CURDIR)/x86_64-opensuse-$*-rootfs addrepo --no-gpgcheck support/opensuse-$*-repo-oss.repo"
	bash iamroot-shell -c "zypper --root $(CURDIR)/x86_64-opensuse-$*-rootfs --non-interactive --no-gpg-checks install patterns-base-minimal_base zypper systemd"

$(eval $(call run,x86_64,opensuse-leap))
$(eval $(call run,x86_64,opensuse-tumbleweed))

qemu-system-x86_64-opensuse-leap: override CMDLINE += rw init=/usr/lib/systemd/systemd
qemu-system-x86_64-opensuse-tumbleweed: override CMDLINE += rw

$(eval $(call zypper-postrootfs,x86_64,opensuse-leap))
$(eval $(call zypper-postrootfs,x86_64,opensuse-tumbleweed))
x86_64-opensuse-leap-postrootfs: export IAMROOT_LD_PRELOAD_LINUX_X86_64_2 = /lib64/libc.so.6:/lib64/libdl.so.2
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

alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.17.0-x86_64.tar.gz
	mkdir -p alpine-minirootfs
	tar xf alpine-minirootfs-3.17.0-x86_64.tar.gz -C alpine-minirootfs

alpine-minirootfs-3.17.0-x86_64.tar.gz:
alpine-minirootfs-%-x86_64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v$(basename $*)/releases/x86_64/alpine-minirootfs-$*-x86_64.tar.gz

.PHONY: mini-chroot-i686
x86-mini-chroot: export QEMU_LD_PREFIX = $(CURDIR)/x86-alpine-minirootfs
x86-mini-chroot: i686/libiamroot-musl-i386.so.1 x86_64/libiamroot-linux-x86-64.so.2 | x86-alpine-minirootfs
	bash iamroot-shell -c "chroot x86-alpine-minirootfs /bin/sh"

.PHONY: x86-alpine-minirootfs
x86-alpine-minirootfs: export QEMU_LD_PREFIX = $(CURDIR)/x86-alpine-minirootfs
x86-alpine-minirootfs: | x86-alpine-minirootfs/bin/busybox

x86-alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.17.0-x86.tar.gz
	mkdir -p x86-alpine-minirootfs
	tar xf alpine-minirootfs-3.17.0-x86.tar.gz -C x86-alpine-minirootfs

alpine-minirootfs-3.17.0-x86.tar.gz:
alpine-minirootfs-%-x86.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v$(basename $*)/releases/x86/alpine-minirootfs-$*-x86.tar.gz

ifneq ($(shell command -v arm-linux-musleabihf-gcc 2>/dev/null),)
arm-rootfs: armhf-alpine-rootfs

.PHONY: mini-chroot
armhf-arch-chroot: export QEMU_LD_PREFIX = $(CURDIR)/armhf-alpine-minirootfs
armhf-mini-chroot: armhf/libiamroot-musl-armhf.so.1 x86_64/libiamroot-linux-x86-64.so.2 | armhf-alpine-minirootfs
	bash iamroot-shell -c "chroot armhf-alpine-minirootfs /bin/sh"

.PHONY: armhf-alpine-minirootfs
armhf-alpine-minirootfs: | armhf-alpine-minirootfs/bin/busybox

armhf-alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.17.0-armhf.tar.gz
	mkdir -p armhf-alpine-minirootfs
	tar xf alpine-minirootfs-3.17.0-armhf.tar.gz -C armhf-alpine-minirootfs

alpine-minirootfs-3.17.0-armhf.tar.gz:
alpine-minirootfs-%-armhf.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v$(basename $*)/releases/armhf/alpine-minirootfs-$*-armhf.tar.gz
endif

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
x86_64-alpine-3.14-chroot:
x86_64-alpine-3.15-chroot:
x86_64-alpine-3.16-chroot:
x86_64-alpine-3.17-chroot:
x86_64-alpine-edge-chroot:
x86_64-alpine-%-chroot: | x86_64-alpine-%-rootfs
	bash iamroot-shell -c "chroot x86_64-alpine-$*-rootfs /bin/ash"

.PHONY: alpine-rootfs
alpine-rootfs: x86_64-alpine-3.14-rootfs
alpine-rootfs: x86_64-alpine-3.15-rootfs
alpine-rootfs: x86_64-alpine-3.16-rootfs
alpine-rootfs: x86_64-alpine-3.17-rootfs
alpine-rootfs: x86_64-alpine-edge-rootfs

x86_64-alpine-3.14-rootfs: | x86_64-alpine-3.14-rootfs/bin/busybox
x86_64-alpine-3.15-rootfs: | x86_64-alpine-3.15-rootfs/bin/busybox
x86_64-alpine-3.16-rootfs: | x86_64-alpine-3.16-rootfs/bin/busybox
x86_64-alpine-3.17-rootfs: | x86_64-alpine-3.17-rootfs/bin/busybox
x86_64-alpine-edge-rootfs: | x86_64-alpine-edge-rootfs/bin/busybox

x86_64-alpine-%-rootfs/bin/busybox: | x86_64/libiamroot-musl-x86_64.so.1 x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "alpine-make-rootfs x86_64-alpine-$*-rootfs --keys-dir /usr/share/apk/keys/x86_64 --mirror-uri http://dl-cdn.alpinelinux.org/alpine --branch $*"

$(eval $(call run,x86_64,alpine-3.14))
$(eval $(call run,x86_64,alpine-3.15))
$(eval $(call run,x86_64,alpine-3.16))
$(eval $(call run,x86_64,alpine-3.17))
$(eval $(call run,x86_64,alpine-edge))

$(eval $(call alpine-postrootfs,x86_64,alpine-3.14))
$(eval $(call alpine-postrootfs,x86_64,alpine-3.15))
$(eval $(call alpine-postrootfs,x86_64,alpine-3.16))
$(eval $(call alpine-postrootfs,x86_64,alpine-3.17))
$(eval $(call alpine-postrootfs,x86_64,alpine-edge))

chroot-alpine-%: PATH = /usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin
chroot-alpine-%: SHELL = /bin/sh

.PRECIOUS: gcompat/ld-%
gcompat/ld-%: LOADER_NAME=ld-$*
gcompat/ld-%: | gcompat/Makefile
	$(MAKE) -C gcompat $(@F) LOADER_NAME=ld-$* CC=musl-gcc

.PRECIOUS: gcompat/libgcompat.so.0
gcompat/libgcompat.so.0: | gcompat/Makefile
	$(MAKE) -C gcompat libgcompat.so.0 CC=musl-gcc LDFLAGS+=-nostdlib

.PRECIOUS: gcompat/Makefile
gcompat/Makefile:
	git clone https://git.adelielinux.org/adelie/gcompat.git

clean: clean-gcompat

.PHONY: clean-gcompat
clean-gcompat:
	-$(MAKE) -C gcompat clean
	rm -f gcompat/ld-*.so*

ifdef COVERAGE
mini-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
alpine-minirootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
alpine-%-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
alpine-%-rootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
IAMROOT_LIB_MUSL_X86_64_1 := $(IAMROOT_LIB_MUSL_X86_64_1):$(CURDIR)/gcompat/libgcompat.so.0
x86_64/libiamroot-musl-x86_64.so.1: | gcompat/libgcompat.so.0
endif

ifneq ($(shell command -v i386-musl-gcc 2>/dev/null),)
x86-alpine-3.14-chroot:
x86-alpine-3.15-chroot:
x86-alpine-3.16-chroot:
x86-alpine-3.17-chroot:
x86-alpine-edge-chroot:
x86-alpine-%-chroot: | i686-alpine-%-rootfs
	bash iamroot-shell -c "chroot x86-alpine-$*-rootfs /bin/ash"

i686-rootfs: x86-alpine-rootfs

.PHONY: x86-alpine-rootfs
x86-alpine-rootfs: x86-alpine-3.14-rootfs
x86-alpine-rootfs: x86-alpine-3.15-rootfs
x86-alpine-rootfs: x86-alpine-3.16-rootfs
x86-alpine-rootfs: x86-alpine-3.17-rootfs
x86-alpine-rootfs: x86-alpine-edge-rootfs

x86-alpine-3.14-rootfs: | x86-alpine-3.14-rootfs/bin/busybox
x86-alpine-3.15-rootfs: | x86-alpine-3.15-rootfs/bin/busybox
x86-alpine-3.16-rootfs: | x86-alpine-3.16-rootfs/bin/busybox
x86-alpine-3.17-rootfs: | x86-alpine-3.17-rootfs/bin/busybox
x86-alpine-edge-rootfs: | x86-alpine-edge-rootfs/bin/busybox

x86-alpine-%-rootfs/bin/busybox: export APK_OPTS = --arch x86 --no-progress
x86-alpine-%-rootfs/bin/busybox: | i686/libiamroot-musl-i386.so.1 x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "alpine-make-rootfs x86-alpine-$*-rootfs --keys-dir /usr/share/apk/keys/x86 --mirror-uri http://dl-cdn.alpinelinux.org/alpine --branch $*"
endif
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
aarch64-arch-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
aarch64-arch-rootfs/etc/machine-id: export EUID = 0
aarch64-arch-rootfs/etc/machine-id: | aarch64/libiamroot-linux-aarch64.so.1 x86_64/libiamroot-linux-x86-64.so.2
	mkdir aarch64-arch-rootfs
	bash iamroot-shell -c "pacstrap -GMC support/aarch64-arch-pacman.conf aarch64-arch-rootfs"
endif

ifneq ($(shell command -v arm-linux-gnueabihf-gcc 2>/dev/null),)
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
armv7h-arch-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
armv7h-arch-rootfs/etc/machine-id: export EUID = 0
armv7h-arch-rootfs/etc/machine-id: | armhf/libiamroot-linux-armhf.so.3 x86_64/libiamroot-linux-x86-64.so.2
	mkdir armv7h-arch-rootfs
	bash iamroot-shell -c "pacstrap -GMC support/armv7h-arch-pacman.conf armv7h-arch-rootfs"
endif
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
aarch64-fedora-33-chroot:
aarch64-fedora-34-chroot:
aarch64-fedora-35-chroot:
aarch64-fedora-36-chroot:
aarch64-fedora-37-chroot:
aarch64-fedora-%-chroot: export IAMROOT_LIBRARY_PATH = /lib64:/usr/lib64
aarch64-fedora-%-chroot: | aarch64-fedora-%-rootfs
	bash iamroot-shell -c "chroot aarch64-fedora-$*-rootfs"

aarch64-rootfs: aarch64-fedora-rootfs

.PHONY: aarch64-fedora-rootfs
aarch64-fedora-rootfs: aarch64-fedora-33-rootfs
aarch64-fedora-rootfs: aarch64-fedora-34-rootfs
aarch64-fedora-rootfs: aarch64-fedora-35-rootfs
aarch64-fedora-rootfs: aarch64-fedora-36-rootfs
aarch64-fedora-rootfs: aarch64-fedora-37-rootfs

aarch64-fedora-33-rootfs: | aarch64-fedora-33-rootfs/etc/machine-id
aarch64-fedora-34-rootfs: | aarch64-fedora-34-rootfs/etc/machine-id
aarch64-fedora-35-rootfs: | aarch64-fedora-35-rootfs/etc/machine-id
aarch64-fedora-36-rootfs: | aarch64-fedora-36-rootfs/etc/machine-id
aarch64-fedora-37-rootfs: | aarch64-fedora-37-rootfs/etc/machine-id

aarch64-fedora-33-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib64/ldb/modules/ldb/tdb.so:/usr/lib64/ldb/modules/ldb/mdb.so:/usr/lib64/ldb/modules/ldb/ldb.so
aarch64-fedora-33-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
aarch64-fedora-33-rootfs/etc/machine-id:
aarch64-fedora-34-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib64/ldb/modules/ldb/tdb.so:/usr/lib64/ldb/modules/ldb/mdb.so:/usr/lib64/ldb/modules/ldb/ldb.so
aarch64-fedora-34-rootfs/etc/machine-id:
aarch64-fedora-35-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
aarch64-fedora-35-rootfs/etc/machine-id:

aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib64/ldb:/lib64:/usr/lib64
aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/aarch64-fedora-$*-rootfs/var/log/dnf.rpm.log
aarch64-fedora-%-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora.repo
aarch64-fedora-%-rootfs/etc/machine-id: | aarch64/libiamroot-linux-aarch64.so.1 x86_64/libiamroot-linux-x86-64.so.2
	install -D -m644 $(FEDORA_REPO) aarch64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --forcearch aarch64 --releasever $* --assumeyes --installroot $(CURDIR)/aarch64-fedora-$*-rootfs group install minimal-environment"
	rm -f aarch64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
endif

ifneq ($(shell command -v arm-linux-gnueabihf-gcc 2>/dev/null),)
armv7hl-fedora-33-chroot:
armv7hl-fedora-34-chroot:
armv7hl-fedora-35-chroot:
armv7hl-fedora-36-chroot:
armv7hl-fedora-%-chroot: | armv7hl-fedora-%-rootfs
	bash iamroot-shell -c "chroot armv7hl-fedora-$*-rootfs"

arm-rootfs: armv7hl-fedora-rootfs

.PHONY: armv7hl-fedora-rootfs
armv7hl-fedora-rootfs: armv7hl-fedora-33-rootfs
armv7hl-fedora-rootfs: armv7hl-fedora-34-rootfs
armv7hl-fedora-rootfs: armv7hl-fedora-35-rootfs
armv7hl-fedora-rootfs: armv7hl-fedora-36-rootfs

armv7hl-fedora-33-rootfs: | armv7hl-fedora-33-rootfs/etc/machine-id
armv7hl-fedora-34-rootfs: | armv7hl-fedora-34-rootfs/etc/machine-id
armv7hl-fedora-35-rootfs: | armv7hl-fedora-35-rootfs/etc/machine-id
armv7hl-fedora-36-rootfs: | armv7hl-fedora-36-rootfs/etc/machine-id

armv7hl-fedora-33-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib/ldb/modules/ldb/tdb.so:/usr/lib/ldb/modules/ldb/mdb.so:/usr/lib/ldb/modules/ldb/ldb.so
armv7hl-fedora-33-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
armv7hl-fedora-33-rootfs/etc/machine-id:
armv7hl-fedora-34-rootfs/etc/machine-id: export IAMROOT_INHIBIT_RPATH = /usr/lib/ldb/modules/ldb/tdb.so:/usr/lib/ldb/modules/ldb/mdb.so:/usr/lib/ldb/modules/ldb/ldb.so
armv7hl-fedora-34-rootfs/etc/machine-id:
armv7hl-fedora-35-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
armv7hl-fedora-35-rootfs/etc/machine-id:

armv7hl-fedora-%-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib/ldb:/lib:/usr/lib
armv7hl-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda|^$(CURDIR)/armv7hl-fedora-$*-rootfs/var/log/dnf.rpm.log
armv7hl-fedora-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_ARMHF_3 = /usr/lib/libc.so.6:/usr/lib/libdl.so.2:/usr/lib/libpthread.so.0:/usr/lib/librt.so.1
armv7hl-fedora-%-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora.repo
armv7hl-fedora-%-rootfs/etc/machine-id: | armhf/libiamroot-linux-armhf.so.3 x86_64/libiamroot-linux-x86-64.so.2
	install -D -m644 $(FEDORA_REPO) armv7hl-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --forcearch armv7hl --releasever $* --assumeyes --installroot $(CURDIR)/armv7hl-fedora-$*-rootfs group install minimal-environment"
	rm -f armv7hl-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
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

aarch64-alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.17.0-aarch64.tar.gz
	mkdir -p aarch64-alpine-minirootfs
	tar xf alpine-minirootfs-3.17.0-aarch64.tar.gz -C aarch64-alpine-minirootfs

alpine-minirootfs-3.13.0-aarch64.tar.gz:
alpine-minirootfs-%-aarch64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v$(basename $*)/releases/aarch64/alpine-minirootfs-$*-aarch64.tar.gz

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
aarch64-alpine-3.14-chroot:
aarch64-alpine-3.15-chroot:
aarch64-alpine-3.16-chroot:
aarch64-alpine-3.17-chroot:
aarch64-alpine-edge-chroot:
aarch64-alpine-%-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-alpine-$*-rootfs
aarch64-alpine-%-chroot: | aarch64-alpine-%-rootfs
	bash iamroot-shell -c "chroot aarch64-alpine-$*-rootfs"

.PHONY: aarch64-alpine-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.14-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.15-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.16-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.17-rootfs
aarch64-alpine-rootfs: aarch64-alpine-edge-rootfs

aarch64-alpine-3.14-rootfs: | aarch64-alpine-3.14-rootfs/bin/busybox
aarch64-alpine-3.15-rootfs: | aarch64-alpine-3.15-rootfs/bin/busybox
aarch64-alpine-3.16-rootfs: | aarch64-alpine-3.16-rootfs/bin/busybox
aarch64-alpine-3.17-rootfs: | aarch64-alpine-3.17-rootfs/bin/busybox
aarch64-alpine-edge-rootfs: | aarch64-alpine-edge-rootfs/bin/busybox

aarch64-alpine-%-rootfs/bin/busybox: | aarch64/libiamroot-musl-aarch64.so.1 x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "APK_OPTS='--arch aarch64' alpine-make-rootfs aarch64-alpine-$*-rootfs --keys-dir /usr/share/apk/keys/aarch64 --mirror-uri http://dl-cdn.alpinelinux.org/alpine --branch $*"
endif
endif

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

mnt:
	mkdir -p $@

.PHONY: support
support: all

.PHONY: extra-support
extra-support: all

.PHONY: fixme-support
fixme-support: all

.PHONY: log
log: all

.PHONY: extra-log
extra-log: all

.PHONY: fixme-log
fixme-log: all

ifneq ($(shell command -v pacstrap 2>/dev/null),)
support: arch-support

.PHONY: arch-support
arch-support: support/x86_64-arch-rootfs.txt
arch-support: support/i686-arch-rootfs.txt

extra-support: manjaro-support

.PHONY: manjaro-support
manjaro-support: support/x86_64-manjaro-stable-rootfs.txt

.PRECIOUS: support/x86_64-arch-rootfs.txt
support/x86_64-arch-rootfs.txt: x86_64-arch-rootfs.log
	support/pacstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/i686-arch-rootfs.txt
support/i686-arch-rootfs.txt: i686-arch-rootfs.log
	support/pacstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-manjaro-stable-rootfs.txt
support/x86_64-manjaro-stable-rootfs.txt: x86_64-manjaro-stable-rootfs.log
	support/pacstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

log: arch-log

.PHONY: arch-log
arch-log: x86_64-arch-rootfs.log
arch-log: i686-arch-rootfs.log

extra-log: manjaro-log

.PHONY: manjaro-log
manjaro-log: x86_64-manjaro-stable-rootfs.log

x86_64-arch-rootfs.log:
i686-arch-rootfs.log:

x86_64-manjaro-stable-rootfs.log:
endif

ifneq ($(shell command -v debootstrap 2>/dev/null),)
support: debian-support

.PHONY: debian-support
debian-support: support/x86_64-debian-oldoldstable-rootfs.txt
debian-support: support/x86_64-debian-oldstable-rootfs.txt
debian-support: support/x86_64-debian-stable-rootfs.txt
debian-support: support/x86_64-debian-testing-rootfs.txt
debian-support: support/x86_64-debian-unstable-rootfs.txt

.PRECIOUS: support/x86_64-debian-oldoldstable-rootfs.txt
support/x86_64-debian-oldoldstable-rootfs.txt: x86_64-debian-oldoldstable-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-debian-oldstable-rootfs.txt
support/x86_64-debian-oldstable-rootfs.txt: x86_64-debian-oldstable-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-debian-stable-rootfs.txt
support/x86_64-debian-stable-rootfs.txt: x86_64-debian-stable-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-debian-testing-rootfs.txt
support/x86_64-debian-testing-rootfs.txt: x86_64-debian-testing-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-debian-unstable-rootfs.txt
support/x86_64-debian-unstable-rootfs.txt: x86_64-debian-unstable-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

log: debian-log

.PHONY: debian-log
debian-log: x86_64-debian-oldoldstable-rootfs.log
debian-log: x86_64-debian-oldstable-rootfs.log
debian-log: x86_64-debian-stable-rootfs.log
debian-log: x86_64-debian-testing-rootfs.log
debian-log: x86_64-debian-unstable-rootfs.log

x86_64-debian-oldoldstable-rootfs.log:
x86_64-debian-oldstable-rootfs.log:
x86_64-debian-stable-rootfs.log:
x86_64-debian-testing-rootfs.log:
x86_64-debian-unstable-rootfs.log:

support: ubuntu-support

.PHONY: ubuntu-support
ubuntu-support: support/x86_64-ubuntu-trusty-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-xenial-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-bionic-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-focal-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-jammy-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-kinetic-rootfs.txt

.PRECIOUS: support/x86_64-ubuntu-trusty-rootfs.txt
support/x86_64-ubuntu-trusty-rootfs.txt: x86_64-ubuntu-trusty-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-ubuntu-xenial-rootfs.txt
support/x86_64-ubuntu-xenial-rootfs.txt: x86_64-ubuntu-xenial-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-ubuntu-bionic-rootfs.txt
support/x86_64-ubuntu-bionic-rootfs.txt: x86_64-ubuntu-bionic-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-ubuntu-focal-rootfs.txt
support/x86_64-ubuntu-focal-rootfs.txt: x86_64-ubuntu-focal-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-ubuntu-jammy-rootfs.txt
support/x86_64-ubuntu-jammy-rootfs.txt: x86_64-ubuntu-jammy-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-ubuntu-kinetic-rootfs.txt
support/x86_64-ubuntu-kinetic-rootfs.txt: x86_64-ubuntu-kinetic-rootfs.log
	support/debootstrap.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

log: ubuntu-log

.PHONY: ubuntu-log
ubuntu-log: x86_64-ubuntu-trusty-rootfs.log
ubuntu-log: x86_64-ubuntu-xenial-rootfs.log
ubuntu-log: x86_64-ubuntu-bionic-rootfs.log
ubuntu-log: x86_64-ubuntu-focal-rootfs.log
ubuntu-log: x86_64-ubuntu-jammy-rootfs.log
ubuntu-log: x86_64-ubuntu-kinetic-rootfs.log

x86_64-ubuntu-trusty-rootfs.log:
x86_64-ubuntu-xenial-rootfs.log:
x86_64-ubuntu-bionic-rootfs.log:
x86_64-ubuntu-focal-rootfs.log:
x86_64-ubuntu-jammy-rootfs.log:
x86_64-ubuntu-kinetic-rootfs.log:
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
support: fedora-support

.PHONY: fedora-support
fedora-support: support/x86_64-fedora-33-rootfs.txt
fedora-support: support/x86_64-fedora-34-rootfs.txt
fedora-support: support/x86_64-fedora-35-rootfs.txt
fedora-support: support/x86_64-fedora-36-rootfs.txt
fedora-support: support/x86_64-fedora-37-rootfs.txt

.PRECIOUS: support/x86_64-fedora-33-rootfs.txt
support/x86_64-fedora-33-rootfs.txt: x86_64-fedora-33-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-fedora-34-rootfs.txt
support/x86_64-fedora-34-rootfs.txt: x86_64-fedora-34-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-fedora-35-rootfs.txt
support/x86_64-fedora-35-rootfs.txt: x86_64-fedora-35-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-fedora-36-rootfs.txt
support/x86_64-fedora-36-rootfs.txt: x86_64-fedora-36-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-fedora-37-rootfs.txt
support/x86_64-fedora-37-rootfs.txt: x86_64-fedora-37-rootfs.log
	support/dnf.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

log: fedora-log

.PHONY: fedora-log
fedora-log: x86_64-fedora-33-rootfs.log
fedora-log: x86_64-fedora-34-rootfs.log
fedora-log: x86_64-fedora-35-rootfs.log
fedora-log: x86_64-fedora-36-rootfs.log

x86_64-fedora-33-rootfs.log:
x86_64-fedora-34-rootfs.log:
x86_64-fedora-35-rootfs.log:
x86_64-fedora-36-rootfs.log:
x86_64-fedora-37-rootfs.log:
endif

ifneq ($(shell command -v zypper 2>/dev/null),)
extra-support: opensuse-support

.PHONY: opensuse-support
fixme-support: support/x86_64-opensuse-leap-rootfs.txt
opensuse-support: support/x86_64-opensuse-tumbleweed-rootfs.txt

.PRECIOUS: support/x86_64-opensuse-leap-rootfs.txt
support/x86_64-opensuse-leap-rootfs.txt: x86_64-opensuse-leap-rootfs.log
	support/zypper.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-opensuse-tumbleweed-rootfs.txt
support/x86_64-opensuse-tumbleweed-rootfs.txt: x86_64-opensuse-tumbleweed-rootfs.log
	support/zypper.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

extra-log: opensuse-log

.PHONY: opensuse-log
fixme-log: x86_64-opensuse-leap-rootfs.log
opensuse-log: x86_64-opensuse-tumbleweed-rootfs.log

opensuse-leap-rootfs.log:
opensuse-tumbleweed-rootfs.log:
endif

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
support: alpine-support

.PHONY: alpine-support
alpine-support: support/x86_64-alpine-3.14-rootfs.txt
alpine-support: support/x86_64-alpine-3.15-rootfs.txt
alpine-support: support/x86_64-alpine-3.16-rootfs.txt
alpine-support: support/x86_64-alpine-3.17-rootfs.txt
alpine-support: support/x86_64-alpine-edge-rootfs.txt

.PRECIOUS: support/x86_64-alpine-3.14-rootfs.txt
support/x86_64-alpine-3.14-rootfs.txt: x86_64-alpine-3.14-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-alpine-3.15-rootfs.txt
support/x86_64-alpine-3.15-rootfs.txt: x86_64-alpine-3.15-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-alpine-3.16-rootfs.txt
support/x86_64-alpine-3.16-rootfs.txt: x86_64-alpine-3.16-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-alpine-3.17-rootfs.txt
support/x86_64-alpine-3.17-rootfs.txt: x86_64-alpine-3.17-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

.PRECIOUS: support/x86_64-alpine-edge-rootfs.txt
support/x86_64-alpine-edge-rootfs.txt: x86_64-alpine-edge-rootfs.log
	support/alpine-make-rootfs.sed -e 's,$(CURDIR),,g' $< >$@.tmp
	mv $@.tmp $@

log: alpine-log

.PHONY: alpine-log
alpine-log: x86_64-alpine-3.14-rootfs.log
alpine-log: x86_64-alpine-3.15-rootfs.log
alpine-log: x86_64-alpine-3.16-rootfs.log
alpine-log: x86_64-alpine-3.17-rootfs.log
alpine-log: x86_64-alpine-edge-rootfs.log

x86_64-alpine-3.14-rootfs.log:
x86_64-alpine-3.15-rootfs.log:
x86_64-alpine-3.16-rootfs.log:
x86_64-alpine-3.17-rootfs.log:
x86_64-alpine-edge-rootfs.log:
endif

.PHONY: %.log
.PRECIOUS: %.log
%.log: SHELL = /bin/bash -o pipefail
%.log:
	$(MAKE) --silent $* 2>&1 | tee $@.tmp
	mv $@.tmp $@
endif

%:
	$(MAKE) -f Makefile $@
