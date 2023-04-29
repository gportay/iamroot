#
# Copyright 2021-2023 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

PREFIX ?= /usr/local
O ?= output
CCMACH ?= $(shell $(CC) -dumpmachine 2>/dev/null)
LIBC ?= $(if $(findstring musl,$(CCMACH)),musl,linux)
ARCH ?= $(shell uname -m 2>/dev/null)
KVER ?= $(shell uname -r 2>/dev/null)
VMLINUX_KVER ?= $(shell vmlinux --version 2>/dev/null)

IAMROOT_EXEC = $(CURDIR)/exec.sh
export IAMROOT_EXEC

IAMROOT_EXEC_IGNORE = ldd|mountpoint
export IAMROOT_EXEC_IGNORE

IAMROOT_PATH_RESOLUTION_WORKAROUND ?= 1
export IAMROOT_PATH_RESOLUTION_WORKAROUND

ifeq ($(ARCH),x86_64)
ifeq ($(LIBC),musl)
IAMROOT_LIB = $(IAMROOT_LIB_MUSL_X86_64_1)
export IAMROOT_LIB
else
IAMROOT_LIB = $(IAMROOT_LIB_LINUX_X86_64_2)
export IAMROOT_LIB
endif
endif

ifeq ($(ARCH),aarch64)
IAMROOT_LIB = $(IAMROOT_LIB_LINUX_AARCH64_1)
export IAMROOT_LIB
endif

#
# Stolen from buildroot (support/misc/utils.mk)
#
# SPDX-FileCopyrightText: The musl Contributors
#
# SPDX-License-Identifier: MIT
#
# Case conversion macros. This is inspired by the 'up' macro from gmsl
# (http://gmsl.sf.net). It is optimised very heavily because these macros
# are used a lot. It is about 5 times faster than forking a shell and tr.
#
# The caseconvert-helper creates a definition of the case conversion macro.
# After expansion by the outer $(eval ), the UPPERCASE macro is defined as:
# $(strip $(eval __tmp := $(1))  $(eval __tmp := $(subst a,A,$(__tmp))) ... )
# In other words, every letter is substituted one by one.
#
# The caseconvert-helper allows us to create this definition out of the
# [FROM] and [TO] lists, so we don't need to write down every substition
# manually. The uses of $ and $$ quoting are chosen in order to do as
# much expansion as possible up-front.
#
# Note that it would be possible to conceive a slightly more optimal
# implementation that avoids the use of __tmp, but that would be even
# more unreadable and is not worth the effort.

[FROM] := a b c d e f g h i j k l m n o p q r s t u v w x y z - .
[TO]   := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z _ _

define caseconvert-helper
$(1) = $$(strip \
	$$(eval __tmp := $$(1))\
	$(foreach c, $(2),\
		$$(eval __tmp := $$(subst $(word 1,$(subst :, ,$c)),$(word 2,$(subst :, ,$c)),$$(__tmp))))\
	$$(__tmp))
endef

$(eval $(call caseconvert-helper,UPPERCASE,$(join $(addsuffix :,$([FROM])),$([TO]))))

-include local.mk

MAKEFLAGS += --no-print-directory

.PHONY: all
all:

.PHONY: vars
vars:
	@echo export "IAMROOT_LIB=\"$(IAMROOT_LIB)\""
	@echo export "IAMROOT_EXEC=\"$(IAMROOT_EXEC)\""
	@echo export "IAMROOT_EXEC_IGNORE=\"$(IAMROOT_EXEC_IGNORE)\""

define libs
$(strip libiamroot.so \
	$(if $(findstring :$(2):,:x86_64:                        ),$(if $(findstring :$(1):,:musl:),x86_64/libiamroot-musl-x86_64.so.1  ,x86_64/libiamroot-linux-x86-64.so.2  ), \
	$(if $(findstring :$(2):,:armhf: :arm: :armv7hl: :armv7h:),$(if $(findstring :$(1):,:musl:),armhf/libiamroot-musl-armhf.so.1    ,armhf/libiamroot-linux-armhf.so.3    ), \
	$(if $(findstring :$(2):,:x86: :i386: :i686:             ),$(if $(findstring :$(1):,:musl:),i686/libiamroot-musl-i386.so.1      ,i686/libiamroot-linux.so.2           ), \
	$(if $(findstring :$(2):,:aarch64:                       ),$(if $(findstring :$(1):,:musl:),aarch64/libiamroot-musl-aarch64.so.1,aarch64/libiamroot-linux-aarch64.so.1), \
	$(error $(1)-$(2): No such library))))) \
)
endef

define libiamroot_so =
iamroot_lib_$(2)_$(3) = $(1)/libiamroot-$(2).so.$(3)
IAMROOT_LIB_$(call UPPERCASE,$(2))_$(3) = $(CURDIR)/$(1)/libiamroot-$(2).so.$(3)
export IAMROOT_LIB_$(call UPPERCASE,$(2))_$(3)

vars: iamroot_lib_$(2)_$(3)
iamroot_lib_$(2)_$(3):
	@echo export "IAMROOT_LIB_$(call UPPERCASE,$(2))_$(3)=\"$$(IAMROOT_LIB_$(call UPPERCASE,$(2))_$(3))\""

all: $(1)/libiamroot-$(2).so.$(3)
test: $(1)/libiamroot-$(2).so.$(3)

.PRECIOUS: $(1)/libiamroot-$(2).so.$(3)
$(1)/libiamroot-$(2).so.$(3): $(O)-$(1)-$(2)/libiamroot.so
	install -D -m755 $$< $$@

$(O)-$(1)-$(2)/libiamroot.so:

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
	rm -Rf $(O)-$(1)-$(2)/
	rm -Rf $(1)/
endef

define log
.PRECIOUS: support/$(2).txt
support/$(2).txt: $(2).log
	support/$(1).sed -e 's,$(CURDIR),\$$$$ROOT,g' $$< >$$@.tmp
	mv $$@.tmp $$@

.PRECIOUS: $(2).log
$(2).log: SHELL = /bin/bash -o pipefail
$(2).log:
	$(MAKE) --silent $(2) 2>&1 | tee $$@.tmp
	mv $$@.tmp $$@
endef

define chroot_shell
.PHONY: $(1)-$(2)-chroot
$(1)-$(2)-chroot: | $(1)-$(2)-rootfs
	bash iamroot-shell -c "chroot $(1)-$(2)-rootfs $(3)"

.PHONY: $(1)-$(2)-shell
$(1)-$(2)-shell: libiamroot.so
	@echo $(4)
	bash iamroot-shell
endef

define pacstrap-rootfs
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/etc/machine-id: export EUID = 0

$(eval $(call chroot_shell,$(1),$(2),/bin/bash,pacstrap -GMC support/$(1)-$(2)-pacman.conf $(1)-$(2)-rootfs $(3)))

$(1)-$(2)-rootfs: | $(1)-$(2)-rootfs/etc/machine-id
$(1)-$(2)-rootfs/etc/machine-id: | $(call libs,linux,$(1))
	mkdir -p $(1)-$(2)-rootfs
	bash iamroot-shell -c "pacstrap -GMC support/$(1)-$(2)-pacman.conf $(1)-$(2)-rootfs $(3)"

$(eval $(call log,pacstrap,$(1)-$(2)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2))) \
	$(eval $(call pacstrap-postrootfs,$(1),$(2))) \
)
endef

define debootstrap-rootfs
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /lib/$(1)-linux-gnu:/lib:/usr/lib/$(1)-linux-gnu:/usr/lib
# chfn: PAM: Critical error - immediate abort
# adduser: `/usr/bin/chfn -f systemd Network Management systemd-network' returned error code 1. Exiting.
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|chfn
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^/dev/(null|zero|full|random|urandom|tty|console|pts|shm|ptmx)|^$(CURDIR)/.*\.gcda
# debconf: PERL_DL_NONLAZY is not set, if debconf is running from a preinst script, this is not safe
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export IAMROOT_PRESERVE_ENV = PERL_DL_NONLAZY
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export PERL_DL_NONLAZY = 1
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://deb.debian.org/debian
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export DEBOOTSTRAPFLAGS ?= --merged-usr --no-check-gpg

$(eval $(call chroot_shell,$(1),$(2)-$(3),/bin/bash,debootstrap --keep-debootstrap-dir $$(DEBOOTSTRAPFLAGS) $(3) $(1)-$(2)-$(3)-rootfs $$(DEBOOTSTRAP_MIRROR)))

$(1)-$(2)-$(3)-rootfs: | $(1)-$(2)-$(3)-rootfs/etc/machine-id
$(1)-$(2)-$(3)-rootfs/etc/machine-id: | $(call libs,linux,$(1))
	mkdir -p $(1)-$(2)-$(3)-rootfs
	bash iamroot-shell -c "debootstrap --keep-debootstrap-dir $$(DEBOOTSTRAPFLAGS) $(3) $(1)-$(2)-$(3)-rootfs $$(DEBOOTSTRAP_MIRROR)"
	cat $(1)-$(2)-$(3)-rootfs/debootstrap/debootstrap.log
	rm -Rf $(1)-$(2)-$(3)-rootfs/debootstrap/

$(eval $(call log,debootstrap,$(1)-$(2)-$(3)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2)-$(3))) \
	$(eval $(call debootstrap-postrootfs,$(1),$(2)-$(3))) \
)
endef

define dnf-rootfs
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib64/ldb:/lib64:/usr/lib64
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora.repo

$(eval $(call chroot_shell,$(1),$(2)-$(3),/bin/bash,dnf --forcearch $(1) --releasever $(3) --assumeyes --installroot $(CURDIR)/$(1)-$(2)-$(3)-rootfs group install minimal-environment))

$(1)-$(2)-$(3)-rootfs: | $(1)-$(2)-$(3)-rootfs/etc/machine-id
$(1)-$(2)-$(3)-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_WORKAROUND = 0
$(1)-$(2)-$(3)-rootfs/etc/machine-id: | $(call libs,linux,$(1))
	install -D -m644 $$(FEDORA_REPO) $(1)-$(2)-$(3)-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --forcearch $(1) --releasever $(3) --assumeyes --installroot $(CURDIR)/$(1)-$(2)-$(3)-rootfs group install minimal-environment"
	rm -f $(1)-$(2)-$(3)-rootfs/etc/distro.repos.d/fedora.repo

$(eval $(call log,dnf,$(1)-$(2)-$(3)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2)-$(3))) \
	$(eval $(call dnf-postrootfs,$(1),$(2)-$(3))) \
)
endef

define zypper-rootfs
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|/usr/bin/chkstat
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda

$(eval $(call chroot_shell,$(1),$(2),/bin/bash,zypper --root $(CURDIR)/$(1)-$(2)-rootfs --non-interactive --no-gpg-checks install patterns-base-minimal_base zypper systemd))

$(1)-$(2)-rootfs: | $(1)-$(2)-rootfs/etc/machine-id
$(1)-$(2)-rootfs/etc/machine-id: | $(call libs,linux,$(1))
	bash iamroot-shell -c "zypper --root $(CURDIR)/$(1)-$(2)-rootfs addrepo --no-gpgcheck support/$(2)-repo-oss.repo"
	bash iamroot-shell -c "zypper --root $(CURDIR)/$(1)-$(2)-rootfs --non-interactive --no-gpg-checks install patterns-base-minimal_base zypper systemd"

$(eval $(call log,zypper,$(1)-$(2)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2))) \
	$(eval $(call zypper-postrootfs,$(1),$(2))) \
)
endef

define alpine-make-rootfs-rootfs
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/busybox: export APK_OPTS = --arch $(1) --no-progress

$(eval $(call chroot_shell,$(1),$(2)-$(3),/bin/ash,alpine-make-rootfs $(1)-$(2)-$(3)-rootfs --keys-dir /usr/share/apk/keys/$(1) --mirror-uri http://mirrors.edge.kernel.org/alpine --branch $(3)))

$(1)-$(2)-$(3)-rootfs: | $(1)-$(2)-$(3)-rootfs/bin/busybox
$(1)-$(2)-$(3)-rootfs/bin/busybox: | $(call libs,musl,$(1))
	bash iamroot-shell -c "alpine-make-rootfs $(1)-$(2)-$(3)-rootfs --keys-dir /usr/share/apk/keys/$(1) --mirror-uri http://mirrors.edge.kernel.org/alpine --branch $(3) $$(ALPINE_MAKE_ROOTFSFLAGS)"

$(eval $(call log,alpine-make-rootfs,$(1)-$(2)-$(3)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2)-$(3))) \
	$(eval $(call alpine-postrootfs,$(1),$(2)-$(3))) \
)
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
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	sed -e '/^UNKNOWN$$:/d' \
	    -e '/^tty0$$:/d' \
	    -e '1iUNKNOWN' \
	    -e '1itty0' \
	    -i $(1)-$(2)-rootfs/etc/securetty
	sed -e '/^tty1:/itty0::respawn:/sbin/getty 38400 tty0' \
	    -e '/^tty[1-9]:/s,^,#,' \
	    -e '/^#ttyS0:/s,^#,,g' \
	    -i $(1)-$(2)-rootfs/etc/inittab
	chmod +r $(1)-$(2)-rootfs/bin/bbsuid
endef

define void-rootfs
$(eval $(call chroot_shell,$(1),void,/bin/bash,tar xf void-$(1)-ROOTFS-$(2).tar.xz -C $(1)-void-rootfs))
$(1)-void-chroot $(1)-void-shell: | x86_64/libiamroot-linux-x86-64.so.2

$(1)-void-rootfs: | x86_64/libiamroot-linux-x86-64.so.2 void-$(1)-ROOTFS-$(2).tar.xz
	mkdir -p $(1)-void-rootfs
	tar xf void-$(1)-ROOTFS-$(2).tar.xz -C $(1)-void-rootfs

void-$(1)-ROOTFS-$(2).tar.xz:
	wget http://repo-default.voidlinux.org/live/current/void-$(1)-ROOTFS-$(2).tar.xz
endef

define void-musl-rootfs
$(eval $(call chroot_shell,$(1),void-musl,/bin/bash,tar xf void-$(1)-musl-ROOTFS-$(2).tar.xz -C $(1)-void-musl-rootfs))
$(1)-void-musl-chroot $(1)-void-musl-shell: | x86_64/libiamroot-linux-x86-64.so.2 x86_64/libiamroot-musl-x86_64.so.1

$(1)-void-musl-rootfs: | x86_64/libiamroot-linux-x86-64.so.2 x86_64/libiamroot-musl-x86_64.so.1 void-$(1)-musl-ROOTFS-$(2).tar.xz
	mkdir -p $(1)-void-musl-rootfs
	tar xf void-$(1)-musl-ROOTFS-$(2).tar.xz -C $(1)-void-musl-rootfs

void-$(1)-musl-ROOTFS-$(2).tar.xz:
	wget http://repo-default.voidlinux.org/live/current/void-$(1)-musl-ROOTFS-$(2).tar.xz
endef

define alpine-mini-rootfs
$(eval $(call chroot_shell,$(1),alpine-mini,/bin/ash,tar xf alpine-minirootfs-$(2).0-$(1).tar.gz -C $(1)-alpine-mini-rootfs))
$(1)-alpine-mini-chroot $(1)-alpine-mini-shell: | $(call libs,musl,$(1))

$(1)-alpine-mini-rootfs: | $(1)-alpine-mini-rootfs/bin/busybox
$(1)-alpine-mini-rootfs/bin/busybox: | $(call libs,musl,$(1)) alpine-minirootfs-$(2).0-$(1).tar.gz
	mkdir -p $(1)-alpine-mini-rootfs
	tar xf alpine-minirootfs-$(2).0-$(1).tar.gz -C $(1)-alpine-mini-rootfs

alpine-minirootfs-$(2).0-$(1).tar.gz:
	wget http://mirrors.edge.kernel.org/alpine/v$(2)/releases/$(1)/alpine-minirootfs-$(2).0-$(1).tar.gz

$(1)-alpine-mini-rootfs/usr/bin/%: support/% | $(1)-alpine-mini-rootfs
	cp $$< $$@
endef

export CC
export CFLAGS

ifeq ($(ARCH),x86_64)
ifeq ($(LIBC),musl)
libiamroot.so: x86_64/libiamroot-musl-x86_64.so.1
	install -D -m755 $< $@

$(eval $(call libiamroot_so,x86_64,musl-x86_64,1))
else
libiamroot.so: x86_64/libiamroot-linux-x86-64.so.2
	install -D -m755 $< $@

$(eval $(call libiamroot_so,x86_64,linux-x86-64,2))

$(O)-i686-linux/libiamroot.so: override CC += -m32
$(O)-i686-linux/libiamroot.so: override CFLAGS += -fno-stack-protector
$(eval $(call libiamroot_so,i686,linux,2))

ifneq ($(shell command -v arm-linux-gnueabi-gcc 2>/dev/null),)
$(O)-arm-linux/libiamroot.so: CC = arm-linux-gnueabi-gcc
$(eval $(call libiamroot_so,arm,linux,3))
endif

ifneq ($(shell command -v arm-linux-gnueabihf-gcc 2>/dev/null),)
$(O)-armhf-linux-armhf/libiamroot.so: CC = arm-linux-gnueabihf-gcc
$(eval $(call libiamroot_so,armhf,linux-armhf,3))
endif

ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
$(O)-aarch64-linux-aarch64/libiamroot.so: CC = aarch64-linux-gnu-gcc
$(eval $(call libiamroot_so,aarch64,linux-aarch64,1))
endif

ifneq ($(shell command -v i386-musl-gcc 2>/dev/null),)
$(O)-i686-musl-i386/libiamroot.so: CC = i386-musl-gcc
$(O)-i686-musl-i386/libiamroot.so: override CFLAGS += -fno-stack-protector
$(eval $(call libiamroot_so,i686,musl-i386,1))
endif

ifneq ($(shell command -v musl-gcc 2>/dev/null),)
$(O)-x86_64-musl-x86_64/libiamroot.so: CC = musl-gcc
$(eval $(call libiamroot_so,x86_64,musl-x86_64,1))
endif

ifneq ($(shell command -v arm-linux-musleabihf-gcc 2>/dev/null),)
$(O)-armhf-musl-armhf/libiamroot.so: CC = arm-linux-musleabihf-gcc
$(eval $(call libiamroot_so,armhf,musl-armhf,1))
endif

ifneq ($(shell command -v aarch64-linux-musl-gcc 2>/dev/null),)
$(O)-aarch64-musl-aarch64/libiamroot.so: CC = aarch64-linux-musl-gcc
$(eval $(call libiamroot_so,aarch64,musl-aarch64,1))
endif
endif

ifdef CLANG
ifneq ($(shell command -v clang 2>/dev/null),)
all: clang/libiamroot-linux-x86-64.so.2

.PRECIOUS: clang/libiamroot-linux-x86_64.so.2
clang/libiamroot-linux-x86-64.so.2: $(O)-clang-linux-x86-64/libiamroot.so

$(O)-clang-linux-x86-64/libiamroot.so:

clang/libiamroot-linux-x86-64.so.2:
clang/libiamroot-linux-x86-64.so.2: export CC = clang

clean: clean-clang-linux-x86-64.2

.PHONY: clean-clang-linux-x86-64.2
clean-clang-linux-x86-64.2:
	rm -Rf $(O)-clang-x86-64/
	rm -Rf clang/
endif
endif
endif

ifeq ($(ARCH),aarch64)
libiamroot.so: aarch64/libiamroot-linux-aarch64.so.1
	install -D -m755 $< $@

$(eval $(call libiamroot_so,aarch64,linux-aarch64,1))
endif

.PRECIOUS: $(O)-%/libiamroot.so
$(O)-%/libiamroot.so: $(wildcard *.c) | $(O)-%
	$(MAKE) -f $(CURDIR)/Makefile -C $(O)-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: $(O)-%
$(O)-%:
	mkdir -p $@

.PHONY: rootfs
rootfs: i686-rootfs

.PHONY: extra-rootfs
extra-rootfs:

.PHONY: broken-rootfs
broken-rootfs:

.PHONY: fixme-rootfs
fixme-rootfs:

.PHONY: i686-rootfs
i686-rootfs:

.PHONY: aarch64-rootfs
aarch64-rootfs:

.PHONY: arm-rootfs
arm-rootfs:

.PHONY: ci
ci: test
	$(MAKE) -f Makefile $@

.PHONY: test
test:
	$(MAKE) -f Makefile $@

.PHONY: coverage
coverage: gcov/index.html

.PHONY: gcov/index.html
gcov/index.html:
	mkdir -p $(@D)
	gcovr --html-details --html-title iamroot -s -v -o $@ $(O)-x86_64-linux-x86-64/ tests/

.PHONY: cobertura.xml
cobertura.xml:
	gcovr --cobertura -s -v -o $@ $(O)-x86_64-linux-x86-64/ tests/

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

ifeq ($(ARCH),x86_64)
ifneq ($(shell command -v pacstrap 2>/dev/null),)
.PHONY: archlinux-test
archlinux-test: | x86_64-archlinux-rootfs/usr/bin/shebang.sh
archlinux-test: | x86_64-archlinux-rootfs/usr/bin/shebang-arg.sh
archlinux-test: $(subst $(CURDIR)/,,$(IAMROOT_LIB)) | x86_64-archlinux-rootfs
	bash iamroot-shell -c "chroot x86_64-archlinux-rootfs shebang.sh one two three"
	bash iamroot-shell -c "chroot x86_64-archlinux-rootfs shebang-arg.sh one two three"

x86_64-archlinux-rootfs/usr/bin/%: support/% | x86_64-archlinux-rootfs
	cp $< $@

rootfs: x86_64-archlinux-rootfs

$(eval $(call pacstrap-rootfs,x86_64,archlinux,base))

i686-rootfs: i686-archlinux32-rootfs

.PHONY: i686-archlinux32-rootfs
i686-archlinux32-rootfs: | i686-archlinux32-rootfs/etc/machine-id

$(eval $(call pacstrap-rootfs,i686,archlinux32,base))

extra-rootfs: x86_64-manjaro-stable-rootfs

.PHONY: x86_64-manjaro-stable-rootfs
x86_64-manjaro-stable-rootfs: | x86_64-manjaro-stable-rootfs/etc/machine-id

$(eval $(call pacstrap-rootfs,x86_64,manjaro-stable,base))
endif

ifneq ($(shell command -v debootstrap 2>/dev/null),)
rootfs: x86_64-debian-rootfs

.PHONY: x86_64-debian-rootfs
x86_64-debian-rootfs: x86_64-debian-oldstable-rootfs
x86_64-debian-rootfs: x86_64-debian-stable-rootfs
x86_64-debian-rootfs: x86_64-debian-testing-rootfs
x86_64-debian-rootfs: x86_64-debian-unstable-rootfs

$(eval $(call debootstrap-rootfs,x86_64,debian,oldstable))
$(eval $(call debootstrap-rootfs,x86_64,debian,stable))
$(eval $(call debootstrap-rootfs,x86_64,debian,testing))
$(eval $(call debootstrap-rootfs,x86_64,debian,unstable))

.PHONY: x86_64-ubuntu-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-trusty-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-xenial-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-bionic-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-focal-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-jammy-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-kinetic-rootfs
x86_64-ubuntu-rootfs: x86_64-ubuntu-lunar-rootfs

x86_64-ubuntu-trusty-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
x86_64-ubuntu-xenial-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
x86_64-ubuntu-bionic-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
x86_64-ubuntu-focal-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
x86_64-ubuntu-jammy-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
x86_64-ubuntu-kinetic-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
x86_64-ubuntu-lunar-rootfs/etc/machine-id: export DEBOOTSTRAP_MIRROR ?= http://archive.ubuntu.com/ubuntu
$(eval $(call debootstrap-rootfs,x86_64,ubuntu,trusty))
$(eval $(call debootstrap-rootfs,x86_64,ubuntu,xenial))
$(eval $(call debootstrap-rootfs,x86_64,ubuntu,bionic))
$(eval $(call debootstrap-rootfs,x86_64,ubuntu,focal))
$(eval $(call debootstrap-rootfs,x86_64,ubuntu,jammy))
$(eval $(call debootstrap-rootfs,x86_64,ubuntu,kinetic))
$(eval $(call debootstrap-rootfs,x86_64,ubuntu,lunar))
# chmod: cannot access '/dev/tty[0-9]*': No such file or directory
# dpkg: error processing package makedev (--configure):
#  subprocess installed post-installation script returned error exit status 1
x86_64-ubuntu-trusty-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|/var/lib/dpkg/info/(initscripts|initramfs-tools|makedev).postinst
x86_64-ubuntu-xenial-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|chfn|/var/lib/dpkg/info/(initramfs-tools|makedev).postinst
x86_64-ubuntu-bionic-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|pam-auth-update|chfn|/var/lib/dpkg/info/initramfs-tools.postinst
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
x86_64-ubuntu-trusty-rootfs/etc/machine-id: export IAMROOT_PRESERVE_ENV := $(IAMROOT_PRESERVE_ENV):LDCONFIG_NOTRIGGER
x86_64-ubuntu-trusty-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
x86_64-ubuntu-xenial-rootfs/etc/machine-id: export IAMROOT_PRESERVE_ENV := $(IAMROOT_PRESERVE_ENV):LDCONFIG_NOTRIGGER
x86_64-ubuntu-xenial-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
x86_64-ubuntu-bionic-rootfs/etc/machine-id: export IAMROOT_PRESERVE_ENV := $(IAMROOT_PRESERVE_ENV):LDCONFIG_NOTRIGGER
x86_64-ubuntu-bionic-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
x86_64-ubuntu-focal-rootfs/etc/machine-id: export IAMROOT_PRESERVE_ENV := $(IAMROOT_PRESERVE_ENV):LDCONFIG_NOTRIGGER
x86_64-ubuntu-focal-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
x86_64-ubuntu-jammy-rootfs/etc/machine-id: export IAMROOT_PRESERVE_ENV := $(IAMROOT_PRESERVE_ENV):LDCONFIG_NOTRIGGER
x86_64-ubuntu-jammy-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
x86_64-ubuntu-kinetic-rootfs/etc/machine-id: export IAMROOT_PRESERVE_ENV := $(IAMROOT_PRESERVE_ENV):LDCONFIG_NOTRIGGER
x86_64-ubuntu-kinetic-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
x86_64-ubuntu-lunar-rootfs/etc/machine-id: export IAMROOT_PRESERVE_ENV := $(IAMROOT_PRESERVE_ENV):LDCONFIG_NOTRIGGER
x86_64-ubuntu-lunar-rootfs/etc/machine-id: export LDCONFIG_NOTRIGGER = y
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
rootfs: x86_64-fedora-rootfs

.PHONY: x86_64-fedora-rootfs
fedora-rootfs: x86_64-fedora-33-rootfs
fedora-rootfs: x86_64-fedora-34-rootfs
fedora-rootfs: x86_64-fedora-35-rootfs
fedora-rootfs: x86_64-fedora-36-rootfs
fedora-rootfs: x86_64-fedora-37-rootfs
fedora-rootfs: x86_64-fedora-38-rootfs

x86_64-fedora-33-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-35-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
$(eval $(call dnf-rootfs,x86_64,fedora,33))
$(eval $(call dnf-rootfs,x86_64,fedora,34))
$(eval $(call dnf-rootfs,x86_64,fedora,35))
$(eval $(call dnf-rootfs,x86_64,fedora,36))
$(eval $(call dnf-rootfs,x86_64,fedora,37))
$(eval $(call dnf-rootfs,x86_64,fedora,38))
endif

ifneq ($(shell command -v zypper 2>/dev/null),)
x86_64-opensuse-leap-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_WORKAROUND = 0

extra-rootfs: opensuse-rootfs

.PHONY: opensuse-rootfs
opensuse-rootfs: | x86_64-opensuse-tumbleweed-rootfs

$(eval $(call zypper-rootfs,x86_64,opensuse-leap))
$(eval $(call zypper-rootfs,x86_64,opensuse-tumbleweed))
x86_64-opensuse-leap-chroot x86_64-opensuse-leap-shell x86_64-opensuse-leap-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd|mountpoint|/usr/bin/chkstat|/usr/sbin/update-ca-certificates

qemu-system-x86_64-opensuse-leap: override CMDLINE += rw init=/usr/lib/systemd/systemd
endif

$(eval $(call void-rootfs,x86_64,20221001))

ifneq ($(shell command -v musl-gcc 2>/dev/null)$(if musl,$(LIBC),YES,),)
.PHONY: alpine-test
alpine-test: | x86_64-alpine-mini-rootfs/usr/bin/shebang.sh
alpine-test: | x86_64-alpine-mini-rootfs/usr/bin/shebang-arg.sh
alpine-test: | x86_64-alpine-mini-rootfs/usr/bin/shebang-busybox.sh
alpine-test: $(call libs,musl,x86_64) | x86_64-alpine-mini-rootfs
	bash iamroot-shell -c "chroot x86_64-alpine-mini-rootfs pwd" | tee /dev/stderr | grep -q "^/\$$"
	bash iamroot-shell -c "chroot x86_64-alpine-mini-rootfs cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell -c "chroot x86_64-alpine-mini-rootfs chroot . cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell -c "chroot x86_64-alpine-mini-rootfs /bin/busybox"
	bash iamroot-shell -c "chroot x86_64-alpine-mini-rootfs shebang.sh one two three"
	bash iamroot-shell -c "chroot x86_64-alpine-mini-rootfs shebang-arg.sh one two three"
	bash iamroot-shell -c "chroot x86_64-alpine-mini-rootfs shebang-busybox.sh one two three"
	bash iamroot-shell -c "chroot x86_64-alpine-mini-rootfs /lib/ld-musl-x86_64.so.1 --preload "$$PWD/x86_64/libiamroot-musl-x86_64.so.1" bin/busybox"

rootfs: alpine-rootfs

$(eval $(call void-musl-rootfs,x86_64,20221001))

$(eval $(call alpine-mini-rootfs,x86_64,3.17))

$(eval $(call alpine-mini-rootfs,x86,3.17))

ifneq ($(shell command -v arm-linux-musleabihf-gcc 2>/dev/null),)
arm-rootfs: armhf-alpine-rootfs

$(eval $(call alpine-mini-rootfs,armhf,3.17))
endif

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
.PHONY: alpine-rootfs
alpine-rootfs: x86_64-alpine-3.14-rootfs
alpine-rootfs: x86_64-alpine-3.15-rootfs
alpine-rootfs: x86_64-alpine-3.16-rootfs
alpine-rootfs: x86_64-alpine-3.17-rootfs
alpine-rootfs: x86_64-alpine-3.18-rootfs
alpine-rootfs: x86_64-alpine-edge-rootfs

x86_64-alpine-edge-rootfs/bin/busybox: ALPINE_MAKE_ROOTFSFLAGS = --packages apk-tools --packages openrc
$(eval $(call alpine-make-rootfs-rootfs,x86_64,alpine,3.14))
$(eval $(call alpine-make-rootfs-rootfs,x86_64,alpine,3.15))
$(eval $(call alpine-make-rootfs-rootfs,x86_64,alpine,3.16))
$(eval $(call alpine-make-rootfs-rootfs,x86_64,alpine,3.17))
$(eval $(call alpine-make-rootfs-rootfs,x86_64,alpine,3.18))
$(eval $(call alpine-make-rootfs-rootfs,x86_64,alpine,edge))

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
x86_64-alpine-mini-shell: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-mini-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-mini-rootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.14-shell: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.14-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.14-rootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.15-shell: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.15-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.15-rootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.16-shell: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.16-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.16-rootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.17-shell: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.17-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.17-rootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.18-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-3.18-rootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-edge-shell: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-edge-chroot: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
x86_64-alpine-edge-rootfs: IAMROOT_LIB := $(IAMROOT_LIB):$(CURDIR)/gcompat/libgcompat.so.0
IAMROOT_LIB_MUSL_X86_64_1 := $(IAMROOT_LIB_MUSL_X86_64_1):$(CURDIR)/gcompat/libgcompat.so.0
x86_64/libiamroot-musl-x86_64.so.1: | gcompat/libgcompat.so.0
endif

ifneq ($(shell command -v i386-musl-gcc 2>/dev/null),)
i686-rootfs: x86-alpine-rootfs

.PHONY: x86-alpine-rootfs
x86-alpine-rootfs: x86-alpine-3.14-rootfs
x86-alpine-rootfs: x86-alpine-3.15-rootfs
x86-alpine-rootfs: x86-alpine-3.16-rootfs
x86-alpine-rootfs: x86-alpine-3.17-rootfs
x86-alpine-rootfs: x86-alpine-3.18-rootfs
x86-alpine-rootfs: x86-alpine-edge-rootfs

$(eval $(call alpine-make-rootfs-rootfs,x86,alpine,3.14))
$(eval $(call alpine-make-rootfs-rootfs,x86,alpine,3.15))
$(eval $(call alpine-make-rootfs-rootfs,x86,alpine,3.16))
$(eval $(call alpine-make-rootfs-rootfs,x86,alpine,3.17))
$(eval $(call alpine-make-rootfs-rootfs,x86,alpine,3.18))
$(eval $(call alpine-make-rootfs-rootfs,x86,alpine,edge))
endif
endif
endif

ifneq ($(shell command -v pacstrap 2>/dev/null),)
ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
aarch64-rootfs: aarch64-archlinuxarm-rootfs

.PHONY: aarch64-archlinuxarm-rootfs
aarch64-archlinuxarm-rootfs: | aarch64-archlinuxarm-rootfs/etc/machine-id

$(eval $(call pacstrap-rootfs,aarch64,archlinuxarm,base))
aarch64-archlinuxarm-chroot aarch64-archlinuxarm-shell aarch64-archlinuxarm-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /lib:/usr/lib
endif

ifneq ($(shell command -v arm-linux-gnueabihf-gcc 2>/dev/null),)
arm-rootfs: armv7h-archlinuxarm-rootfs

.PHONY: armv7h-archlinuxarm-rootfs
armv7h-archlinuxarm-rootfs: | armv7h-archlinuxarm-rootfs/etc/machine-id

$(eval $(call pacstrap-rootfs,armv7h,archlinuxarm,base))
endif
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
aarch64-rootfs: aarch64-fedora-rootfs

.PHONY: aarch64-fedora-rootfs
aarch64-fedora-rootfs: aarch64-fedora-33-rootfs
aarch64-fedora-rootfs: aarch64-fedora-34-rootfs
aarch64-fedora-rootfs: aarch64-fedora-35-rootfs
aarch64-fedora-rootfs: aarch64-fedora-36-rootfs
aarch64-fedora-rootfs: aarch64-fedora-37-rootfs
aarch64-fedora-rootfs: aarch64-fedora-38-rootfs

aarch64-fedora-33-rootfs: | aarch64-fedora-33-rootfs/etc/machine-id
aarch64-fedora-34-rootfs: | aarch64-fedora-34-rootfs/etc/machine-id
aarch64-fedora-35-rootfs: | aarch64-fedora-35-rootfs/etc/machine-id
aarch64-fedora-36-rootfs: | aarch64-fedora-36-rootfs/etc/machine-id
aarch64-fedora-37-rootfs: | aarch64-fedora-37-rootfs/etc/machine-id
aarch64-fedora-38-rootfs: | aarch64-fedora-38-rootfs/etc/machine-id

aarch64-fedora-33-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
aarch64-fedora-35-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
$(eval $(call dnf-rootfs,aarch64,fedora,33))
$(eval $(call dnf-rootfs,aarch64,fedora,34))
$(eval $(call dnf-rootfs,aarch64,fedora,35))
$(eval $(call dnf-rootfs,aarch64,fedora,36))
$(eval $(call dnf-rootfs,aarch64,fedora,37))
$(eval $(call dnf-rootfs,aarch64,fedora,38))
endif

ifneq ($(shell command -v arm-linux-gnueabihf-gcc 2>/dev/null),)
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

armv7hl-fedora-33-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
armv7hl-fedora-35-rootfs/etc/machine-id: export FEDORA_REPO ?= support/fedora-archive.repo
$(eval $(call dnf-rootfs,armv7hl,fedora,33))
$(eval $(call dnf-rootfs,armv7hl,fedora,34))
$(eval $(call dnf-rootfs,armv7hl,fedora,35))
$(eval $(call dnf-rootfs,armv7hl,fedora,36))
armv7hl-fedora-33-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib/ldb:/lib:/usr/lib
armv7hl-fedora-34-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib/ldb:/lib:/usr/lib
armv7hl-fedora-35-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib/ldb:/lib:/usr/lib
armv7hl-fedora-36-rootfs/etc/machine-id: export IAMROOT_LIBRARY_PATH = /usr/lib/ldb:/lib:/usr/lib
endif
endif

ifneq ($(shell command -v aarch64-linux-musl-gcc 2>/dev/null),)
aarch64-rootfs: aarch64-alpine-rootfs

$(eval $(call alpine-mini-rootfs,aarch64,3.17))

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
.PHONY: aarch64-alpine-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.14-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.15-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.16-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.17-rootfs
aarch64-alpine-rootfs: aarch64-alpine-3.18-rootfs
aarch64-alpine-rootfs: aarch64-alpine-edge-rootfs

$(eval $(call alpine-make-rootfs-rootfs,aarch64,alpine,3.14))
$(eval $(call alpine-make-rootfs-rootfs,aarch64,alpine,3.15))
$(eval $(call alpine-make-rootfs-rootfs,aarch64,alpine,3.16))
$(eval $(call alpine-make-rootfs-rootfs,aarch64,alpine,3.17))
$(eval $(call alpine-make-rootfs-rootfs,aarch64,alpine,edge))
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

.PHONY: broken-support
broken-support: all

.PHONY: fixme-support
fixme-support: all

.PHONY: log
log: all

.PHONY: extra-log
extra-log: all

.PHONY: broken-log
broken-log: all

.PHONY: fixme-log
fixme-log: all

ifneq ($(shell command -v pacstrap 2>/dev/null),)
support: archlinux-support

.PHONY: archlinux-support
archlinux-support: support/x86_64-archlinux-rootfs.txt
archlinux-support: support/i686-archlinux32-rootfs.txt

extra-support: manjaro-support

.PHONY: manjaro-support
manjaro-support: support/x86_64-manjaro-stable-rootfs.txt

log: archlinux-log

.PHONY: archlinux-log
archlinux-log: x86_64-archlinux-rootfs.log
archlinux-log: i686-archlinux32-rootfs.log

extra-log: manjaro-log

.PHONY: manjaro-log
manjaro-log: x86_64-manjaro-stable-rootfs.log
endif

ifneq ($(shell command -v debootstrap 2>/dev/null),)
support: debian-support

.PHONY: debian-support
debian-support: support/x86_64-debian-oldstable-rootfs.txt
debian-support: support/x86_64-debian-stable-rootfs.txt
debian-support: support/x86_64-debian-testing-rootfs.txt
debian-support: support/x86_64-debian-unstable-rootfs.txt

log: debian-log

.PHONY: debian-log
debian-log: x86_64-debian-oldstable-rootfs.log
debian-log: x86_64-debian-stable-rootfs.log
debian-log: x86_64-debian-testing-rootfs.log
debian-log: x86_64-debian-unstable-rootfs.log

support: ubuntu-support

.PHONY: ubuntu-support
ubuntu-support: support/x86_64-ubuntu-trusty-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-xenial-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-bionic-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-focal-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-jammy-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-kinetic-rootfs.txt
ubuntu-support: support/x86_64-ubuntu-lunar-rootfs.txt

log: ubuntu-log

.PHONY: ubuntu-log
ubuntu-log: x86_64-ubuntu-trusty-rootfs.log
ubuntu-log: x86_64-ubuntu-xenial-rootfs.log
ubuntu-log: x86_64-ubuntu-bionic-rootfs.log
ubuntu-log: x86_64-ubuntu-focal-rootfs.log
ubuntu-log: x86_64-ubuntu-jammy-rootfs.log
ubuntu-log: x86_64-ubuntu-kinetic-rootfs.log
ubuntu-log: x86_64-ubuntu-lunar-rootfs.log
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
support: fedora-support

.PHONY: fedora-support
fedora-support: support/x86_64-fedora-33-rootfs.txt
fedora-support: support/x86_64-fedora-34-rootfs.txt
fedora-support: support/x86_64-fedora-35-rootfs.txt
fedora-support: support/x86_64-fedora-36-rootfs.txt
fedora-support: support/x86_64-fedora-37-rootfs.txt
fedora-support: support/x86_64-fedora-38-rootfs.txt

log: fedora-log

.PHONY: fedora-log
fedora-log: x86_64-fedora-33-rootfs.log
fedora-log: x86_64-fedora-34-rootfs.log
fedora-log: x86_64-fedora-35-rootfs.log
fedora-log: x86_64-fedora-36-rootfs.log
endif

ifneq ($(shell command -v zypper 2>/dev/null),)
extra-support: opensuse-support

.PHONY: opensuse-support
broken-support: support/x86_64-opensuse-leap-rootfs.txt
opensuse-support: support/x86_64-opensuse-tumbleweed-rootfs.txt

extra-log: opensuse-log

.PHONY: opensuse-log
broken-log: x86_64-opensuse-leap-rootfs.log
opensuse-log: x86_64-opensuse-tumbleweed-rootfs.log
endif

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
support: alpine-support

.PHONY: alpine-support
alpine-support: support/x86_64-alpine-3.14-rootfs.txt
alpine-support: support/x86_64-alpine-3.15-rootfs.txt
alpine-support: support/x86_64-alpine-3.16-rootfs.txt
alpine-support: support/x86_64-alpine-3.17-rootfs.txt
alpine-support: support/x86_64-alpine-3.18-rootfs.txt
alpine-support: support/x86_64-alpine-edge-rootfs.txt

log: alpine-log

.PHONY: alpine-log
alpine-log: x86_64-alpine-3.14-rootfs.log
alpine-log: x86_64-alpine-3.15-rootfs.log
alpine-log: x86_64-alpine-3.16-rootfs.log
alpine-log: x86_64-alpine-3.17-rootfs.log
alpine-log: x86_64-alpine-3.18-rootfs.log
alpine-log: x86_64-alpine-edge-rootfs.log
endif
endif

%:
	$(MAKE) -f Makefile $@
