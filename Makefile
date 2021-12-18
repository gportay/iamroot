#
# Copyright 2020-2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

VERSION = 2
PREFIX ?= /usr/local

%.o: override CFLAGS += -fPIC -Wall -Wextra -Werror
%.o: override CFLAGS += -D_GNU_SOURCE
%.o: override CFLAGS += -DARG_MAX=$(shell getconf ARG_MAX)
%.so: override LDFLAGS += -nolibc

QEMU ?= qemu-system-x86_64
QEMU += -enable-kvm -m 4G -machine q35 -smp 4 -cpu host
ifndef NO_SPICE
QEMU += -vga virtio -display egl-headless,gl=on
QEMU += -spice port=5924,disable-ticketing -device virtio-serial-pci -device virtserialport,chardev=spicechannel0,name=com.redhat.spice.0 -chardev spicevmc,id=spicechannel0,name=vdagent
endif
QEMU += -serial mon:stdio

KVER ?= $(shell uname -r 2>/dev/null)
VMLINUX_KVER ?= $(shell vmlinux --version 2>/dev/null)

.PHONY: all
all: libiamroot-linux-x86-64.so.2

libiamroot-linux-x86-64.so.2: libiamroot.so
	ln -sf $< $@

libiamroot.so: __fperror.o
libiamroot.so: __fstat.o
libiamroot.so: __fstat64.o
libiamroot.so: __fxstat.o
libiamroot.so: __fxstat64.o
libiamroot.so: __fxstatat.o
libiamroot.so: __fxstatat64.o
libiamroot.so: __lxstat.o
libiamroot.so: __lxstat64.o
libiamroot.so: __nss_files_fopen.o
libiamroot.so: __open.o
libiamroot.so: __open64.o
libiamroot.so: __open64_2.o
libiamroot.so: __open_2.o
libiamroot.so: __openat64_2.o
libiamroot.so: __openat_2.o
libiamroot.so: __opendir.o
libiamroot.so: __opendirat.o
libiamroot.so: __perror.o
libiamroot.so: __perror2.o
libiamroot.so: __statfs.o
libiamroot.so: __statfs64.o
libiamroot.so: __verbosef.o
libiamroot.so: __xmknod.o
libiamroot.so: __xmknodat.o
libiamroot.so: __xstat.o
libiamroot.so: __xstat64.o
libiamroot.so: access.o
libiamroot.so: canonicalize_file_name.o
libiamroot.so: chdir.o
libiamroot.so: chmod.o
libiamroot.so: chown.o
libiamroot.so: chroot.o
libiamroot.so: creat.o
libiamroot.so: creat64.o
libiamroot.so: dlmopen.o
libiamroot.so: dlopen.o
libiamroot.so: eaccess.o
libiamroot.so: euidaccess.o
libiamroot.so: execl.o
libiamroot.so: execle.o
libiamroot.so: execlp.o
libiamroot.so: execv.o
libiamroot.so: execve.o
libiamroot.so: execvp.o
libiamroot.so: execvpe.o
libiamroot.so: faccessat.o
libiamroot.so: fchdir.o
libiamroot.so: fchmod.o
libiamroot.so: fchmodat.o
libiamroot.so: fchown.o
libiamroot.so: fchownat.o
libiamroot.so: fexecve.o
libiamroot.so: fgetxattr.o
libiamroot.so: fopen.o
libiamroot.so: fopen64.o
libiamroot.so: fpath_resolutionat.o
libiamroot.so: freopen.o
libiamroot.so: freopen64.o
libiamroot.so: fsetxattr.o
libiamroot.so: fstat.o
libiamroot.so: fstat64.o
libiamroot.so: fstatat.o
libiamroot.so: fstatat64.o
libiamroot.so: futimesat.o
libiamroot.so: get_current_dir_name.o
libiamroot.so: getcwd.o
libiamroot.so: geteuid.o
libiamroot.so: getuid.o
libiamroot.so: getwd.o
libiamroot.so: getxattr.o
libiamroot.so: group.o
libiamroot.so: lchmod.o
libiamroot.so: lchown.o
libiamroot.so: lgetxattr.o
libiamroot.so: link.o
libiamroot.so: linkat.o
libiamroot.so: listxattr.o
libiamroot.so: llistxattr.o
libiamroot.so: lremovexattr.o
libiamroot.so: lsetxattr.o
libiamroot.so: lstat.o
libiamroot.so: lstat64.o
libiamroot.so: lutimes.o
libiamroot.so: mkdir.o
libiamroot.so: mkdirat.o
libiamroot.so: mkdtemp.o
libiamroot.so: mkfifo.o
libiamroot.so: mkfifoat.o
libiamroot.so: mknod.o
libiamroot.so: mknodat.o
libiamroot.so: mkostemp.o
libiamroot.so: mkostemp64.o
libiamroot.so: mkostemps.o
libiamroot.so: mkostemps64.o
libiamroot.so: mkstemp.o
libiamroot.so: mkstemp64.o
libiamroot.so: mkstemps.o
libiamroot.so: mkstemps64.o
libiamroot.so: mktemp.o
libiamroot.so: mount.o
libiamroot.so: name_to_handle_at.o
libiamroot.so: open.o
libiamroot.so: open64.o
libiamroot.so: openat.o
libiamroot.so: openat64.o
libiamroot.so: opendir.o
libiamroot.so: opendir64.o
libiamroot.so: passwd.o
libiamroot.so: path_resolution.o
libiamroot.so: posix_spawn.o
libiamroot.so: posix_spawnp.o
libiamroot.so: readlink.o
libiamroot.so: readlinkat.o
libiamroot.so: realpath.o
libiamroot.so: remove.o
libiamroot.so: removexattr.o
libiamroot.so: rename.o
libiamroot.so: renameat.o
libiamroot.so: renameat2.o
libiamroot.so: rmdir.o
libiamroot.so: running_in_chroot.o
libiamroot.so: scandir.o
libiamroot.so: scandirat.o
libiamroot.so: setegid.o
libiamroot.so: setegid.o
libiamroot.so: setgid.o
libiamroot.so: setuid.o
libiamroot.so: setxattr.o
libiamroot.so: shadow.o
libiamroot.so: stat.o
libiamroot.so: stat64.o
libiamroot.so: statfs.o
libiamroot.so: statfs64.o
libiamroot.so: statvfs.o
libiamroot.so: statvfs64.o
libiamroot.so: statx.o
libiamroot.so: symlink.o
libiamroot.so: symlinkat.o
libiamroot.so: tempnam.o
libiamroot.so: tmpnam.o
libiamroot.so: tmpnam_r.o
libiamroot.so: truncate.o
libiamroot.so: umask.o
libiamroot.so: umount.o
libiamroot.so: umount2.o
libiamroot.so: unlink.o
libiamroot.so: unlinkat.o
libiamroot.so: unshare.o
libiamroot.so: utime.o
libiamroot.so: utimensat.o
libiamroot.so: utimes.o
libiamroot.so: whereami.o
libiamroot.so: whoami.o
libiamroot.so: override LDLIBS += -ldl -lpthread

.PHONY: doc
doc: iamroot-shell.1.gz iamroot.7.gz

.PHONY: install
install: install-exec install-doc install-bash-completion

.PHONY: install-exec
install-exec:
	install -D -m 755 iamroot-shell $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	sed -e "s,\$$PWD,$(PREFIX)/lib/iamroot," -i $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	install -D -m 755 libiamroot.so $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-x86-64.so.2
	ln -sf libiamroot-linux-x86-64.so.2 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	install -D -m 755 exec.sh $(DESTDIR)$(PREFIX)/lib/iamroot/exec.sh

.PHONY: install-doc
install-doc:
	install -D -m 644 iamroot-shell.1.gz $(DESTDIR)$(PREFIX)/share/man/man1/iamroot-shell.1.gz
	install -D -m 644 iamroot.7.gz $(DESTDIR)$(PREFIX)/share/man/man7/iamroot.7.gz

.PHONY: install-bash-completion
install-bash-completion:
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion)}; \
	if [ -n "$$completionsdir" ]; then \
		install -D -m 644 bash-completion $(DESTDIR)$$completionsdir/iamroot-shell; \
	fi

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-x86-64.so.2
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/exec.sh
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/iamroot-shell.1.gz
	rm -f $(DESTDIR)$(PREFIX)/share/man/man7/iamroot.7.gz
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion)}; \
	if [ -n "$$completionsdir" ]; then \
		rm -f $(DESTDIR)$$completionsdir/iamroot-shell; \
	fi

.PHONY: user-install
user-install: user-install-exec user-install-doc user-install-bash-completion

user-install-exec user-install-doc user-install-bash-completion user-uninstall:
user-%:
	$(MAKE) $* PREFIX=$$HOME/.local BASHCOMPLETIONSDIR=$$HOME/.local/share/bash-completion/completions

.PHONY: ci
ci: check test

.PHONY: run-qemu
run-qemu: override QEMUFLAGS += -append "console=ttyS0 root=host0 rootfstype=9p rootflags=trans=virtio debug"
run-qemu: override QEMUFLAGS += -kernel arch-rootfs/boot/vmlinuz-linux
run-qemu: override QEMUFLAGS += -initrd arch-rootfs/boot/initramfs-linux-fallback.img
run-qemu: override QEMUFLAGS += -virtfs local,path=$(CURDIR)/arch-rootfs,mount_tag=host0,security_model=passthrough,id=host0
run-qemu: | arch-rootfs/boot/vmlinuz-linux
	$(QEMU) $(QEMUFLAGS)

.PHONY: check
check:
	shellcheck iamroot-shell exec.sh

.PHONY: test
test: shell-test
test: static-test
test: | libiamroot-linux-x86-64.so.2 alpine-minirootfs
	$(MAKE) -C tests
	$(MAKE) -C tests $@ LD_PRELOAD=$(CURDIR)/libiamroot-linux-x86-64.so.2 IAMROOT_LIB=$(CURDIR)/libiamroot-linux-x86-64.so.2 ALPINE_MINIROOTFS=$(CURDIR)/alpine-minirootfs

.PHONY: static-test
static-test: SHELL = /bin/bash
static-test: libiamroot-linux-x86-64.so.2 | static-rootfs
	bash iamroot-shell -c "whoami | tee /dev/stderr | grep -q \"^root\$$\""
	bash iamroot-shell -c "IAMROOT_EUID=$$EUID whoami | tee /dev/stderr | grep -q \"^$(shell whoami)\$$\""

.PHONY: shell-test
shell-test: libiamroot-linux-x86-64.so.2 | static-rootfs
	bash iamroot-shell -c "whoami" | tee /dev/stderr | grep -q "^root\$$"
	bash iamroot-shell -c "stat -c '%u:%g' ." | tee /dev/stderr | grep -q "^0:0$$"
	bash iamroot-shell -c "echo \$$IAMROOTLVL" | tee /dev/stderr | grep -q "^[0-9]\+$$"

.PHONY: alpine-test
alpine-test: | alpine-minirootfs/usr/bin/shebang.sh
alpine-test: | alpine-minirootfs/usr/bin/shebang-arg.sh
alpine-test: | alpine-minirootfs/usr/bin/shebang-busybox.sh
alpine-test: libiamroot-linux-x86-64.so.2 | alpine-minirootfs
	bash iamroot-shell -c "chroot alpine-minirootfs pwd" | tee /dev/stderr | grep -q "^/\$$"
	bash iamroot-shell -c "chroot alpine-minirootfs cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs chroot . cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell -c "chroot alpine-minirootfs /bin/busybox"
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs shebang.sh one two three"
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs shebang-arg.sh one two three"
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs shebang-busybox.sh one two three"

.PHONY: arch-test
arch-test: | arch-rootfs/usr/bin/shebang.sh
arch-test: | arch-rootfs/usr/bin/shebang-arg.sh
arch-test: | arch-rootfs/usr/bin/shebang-busybox.sh
arch-test: libiamroot-linux-x86-64.so.2 | arch-rootfs/usr/bin/busybox
	bash iamroot-shell -c "chroot arch-rootfs /bin/busybox"
	bash iamroot-shell -c "chroot arch-rootfs shebang.sh one two three"
	bash iamroot-shell -c "chroot arch-rootfs shebang-arg.sh one two three"
	bash iamroot-shell -c "chroot arch-rootfs shebang-busybox.sh one two three"

.PHONY: shell
shell: libiamroot-linux-x86-64.so.2
	bash iamroot-shell

.PHONY: chroot
chroot: libiamroot-linux-x86-64.so.2 | static-rootfs
	bash iamroot-shell -c "chroot static-rootfs/bin/sh"

.PHONY: mini-chroot
mini-chroot: libiamroot-linux-x86-64.so.2 | alpine-minirootfs
	bash iamroot-shell -c "chroot alpine-minirootfs /bin/sh"

alpine-3.14-chroot alpine-edge-chroot:
alpine-%-chroot: | alpine-%-rootfs
	bash iamroot-shell -c "chroot alpine-$*-rootfs /bin/ash"

.PHONY: arch-chroot
arch-chroot: | arch-rootfs
	bash iamroot-shell -c "chroot arch-rootfs"

fedora-34-chroot:
fedora-%-chroot: export IAMROOT_PATH = /usr/bin:/usr/sbin:/bin:/sbin
fedora-%-chroot: export IAMROOT_LD_PRELOAD_LINUX_X86_64 = /usr/lib64/libc.so.6:/usr/lib64/libdl.so.2
fedora-%-chroot: export IAMROOT_LD_LIBRARY_PATH = /usr/lib64:/lib64:/usr/lib64/sssd:/usr/lib:/lib:/usr/lib/systemd
fedora-%-chroot: | fedora-%-rootfs
	bash iamroot-shell -c "chroot fedora-$*-rootfs"

.PHONY: rootfs
rootfs: alpine-3.14-rootfs alpine-edge-rootfs arch-rootfs fedora-34-rootfs

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

alpine-minirootfs/usr/bin/%: support/% | alpine-minirootfs
	cp $< $@

.PHONY: alpine-minirootfs
alpine-minirootfs: | alpine-minirootfs/bin/busybox

alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.13.0-x86_64.tar.gz
	mkdir -p alpine-minirootfs
	tar xf alpine-minirootfs-3.13.0-x86_64.tar.gz -C alpine-minirootfs

alpine-minirootfs-3.13.0-x86_64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v3.13/releases/x86_64/alpine-minirootfs-3.13.0-x86_64.tar.gz

arch-rootfs/usr/bin/%: support/% | arch-rootfs
	cp $< $@

alpine-3.14-rootfs: | alpine-3.14-rootfs/bin/busybox
alpine-edge-rootfs: | alpine-edge-rootfs/bin/busybox

alpine-%-rootfs/bin/busybox: | libiamroot-linux-x86-64.so.2
	bash iamroot-shell -c "alpine-make-rootfs alpine-$*-rootfs --mirror-uri http://nl.alpinelinux.org/alpine --branch $*"

.PHONY: arch-rootfs
arch-rootfs: | arch-rootfs/etc/machine-id

arch-rootfs/usr/bin/busybox: | libiamroot-linux-x86-64.so.2 arch-rootfs/etc/machine-id
	bash iamroot-shell -c "pacman -r arch-rootfs --noconfirm -S busybox"

arch-rootfs/boot/vmlinuz-linux: | libiamroot-linux-x86-64.so.2 arch-rootfs/etc/machine-id
	bash iamroot-shell -c "pacman -r arch-rootfs --noconfirm -S linux"

arch-rootfs/etc/machine-id: | libiamroot-linux-x86-64.so.2
	mkdir -p arch-rootfs
	bash iamroot-shell -c "pacstrap arch-rootfs"

fedora-34-rootfs: | fedora-34-rootfs/etc/machine-id

fedora-34-rootfs/etc/machine-id:
fedora-34-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_X86_64 = /usr/lib64/libc-2.33.so:/usr/lib64/libdl-2.33.so

fedora-%-rootfs/etc/machine-id: export IAMROOT_LD_PRELOAD_LINUX_X86_64 = /usr/lib64/libc.so.6:/usr/lib64/libdl.so.2
fedora-%-rootfs/etc/machine-id: export IAMROOT_LD_LIBRARY_PATH = /usr/lib64:/lib64:/usr/lib64/sssd:/usr/lib:/lib:/usr/lib/systemd
fedora-%-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd
fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|run/.+)/|$(CURDIR)/fedora-$*-rootfs/var/log/dnf.rpm.log
fedora-%-rootfs/etc/machine-id: | libiamroot-linux-x86-64.so.2
	install -D -m644 fedora.repo fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --releasever $* --assumeyes --installroot $(CURDIR)/fedora-$*-rootfs group install minimal-environment"
	rm -f fedora-$*-rootfs/etc/distro.repos.d/fedora.repo

qemu-system-x86_64-alpine-3.14 qemu-system-x86_64-alpine-edge:
qemu-system-x86_64-arch:
qemu-system-x86_64-fedora-34:
qemu-system-x86_64-fedora-34: override CMDLINE += rw
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
vmlinux-alpine-3.14 vmlinux-alpine-edge:
vmlinux-arch:
vmlinux-fedora-34:
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
alpine-3.14.ext4 alpine-edge.ext4:
arch.ext4:
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

arch-postrootfs:
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i arch-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i arch-rootfs/etc/shadow
	mkdir -p arch-rootfs/var/lib/systemd/linger
	rm -f arch-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	bash iamroot-shell --chroot $(CURDIR)/arch-rootfs -c "systemctl enable getty@tty0.service"

fedora-34-postrootfs:
fedora-%-postrootfs: export LD_LIBRARY_PATH = $(CURDIR)/fedora-$*-rootfs/usr/lib/systemd
fedora-%-postrootfs:
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

.PHONY: %-postrootfs
%-postrootfs:

chroot-alpine-%: PATH = /usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin
chroot-alpine-%: SHELL = /bin/sh
chroot-alpine-3.14 chroot-alpine-edge:
chroot-arch:
chroot-%:
	$(MAKE) mount-$*
	-sudo chroot mnt $(SHELL)
	$(MAKE) umount-$*

mount-alpine-3.14 mount-alpine-edge:
mount-arch:
mount-%: | %.ext4 mnt
	sudo mount -oloop $*.ext4 mnt

umount-alpine-3.14 umount-alpine-edge:
umount-arch:
umount-%: | mnt
	sudo umount mnt

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

.PHONY: clean
clean:
	rm -f libiamroot-linux-x86-64.so.2 libiamroot.so *.o *.ext4 *.cpio
	rm -Rf *-rootfs/
	$(MAKE) -C tests $@

.PHONY: mrproper
mrproper: clean
	rm -f busybox-static alpine-minirootfs-3.13.0-x86_64.tar.gz
	rm -Rf busybox/

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.1: %.1.adoc
	asciidoctor -b manpage -o $@ $<

%.7: %.7.adoc
	asciidoctor -b manpage -o $@ $<

%.gz: %
	gzip -c $< >$@
