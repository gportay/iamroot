#!/bin/sh
#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

set -e

# In case of emergency
emergency_shell() {
	set +e
	trap - 0

	echo "Emergency shell..." >&2
	busybox mkdir -p /usr /sbin
	busybox ln -s ../bin /usr/bin
	busybox ln -s ../sbin /usr/sbin
	busybox ln -s ../lib /usr/lib
	busybox --install -s

	export PATH
	exec /bin/sh
}
trap 'emergency_shell' 0

# Setup minimal userspace
busybox mkdir -p /proc /sys
busybox mount -t proc     none /proc
busybox mount -t sysfs    none /sys
busybox mount -t devtmpfs none /dev

# Coldplug devices
busybox mdev -d
busybox find /sys/devices -name modalias -print0 | \
busybox xargs -0 busybox cat | \
busybox xargs    busybox modprobe -a -b -q || \
echo "Warning: Cannot modprobe all modules!" >&2

# Parse command-line
rootoptions=ro
rootfstype=ext4
root=/dev/vda
for word in $(busybox cat /proc/cmdline)
do
	if [ "${word%%=*}" = "$word" ]
	then
		word="$word=1"
	fi
	eval "$(echo "$word")"
done

# Set root options
if [ ! "$ro" ] && [ "$rw" ]
then
	rootoptions=rw
fi

# Wait for root device
while ! test -b "$root"
do
	busybox mdev -s
	busybox sleep 1
done

# Mount root
busybox mkdir /new_root
busybox modprobe -a ext4 crc32c
busybox mount -o "$rootoptions" -t "$rootfstype" "$root" /new_root

# Switch root
busybox mount --move /proc /new_root/proc
busybox mount --move /sys  /new_root/sys
busybox mount --move /dev  /new_root/dev
exec busybox switch_root /new_root /sbin/init "$@"
