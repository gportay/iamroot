#!/bin/sh
#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

log() {
	if [ "${IAMROOT_DEBUG:-0}" -lt 1 ]
	then
		return
	fi

	echo "$@" >&2
}

path="$1"
argv0="${argv0:-$path}"
real_path="$IAMROOT_ROOT$path"
shift

case "${path##*/}" in
mount|umount|systemctl)
	shift
	log "Warning:" "Command is skipped:" "$argv0" "$@"
	;;
ldd|busybox)
	exec "$real_path" "$@"
	;;
ldconfig)
	if [ "${IAMROOT_ROOT:-/}" != / ]
	then
		set -- "$@" -r "$IAMROOT_ROOT"

		# Fixes: $IAMROOT_ROOT/usr/sbin/ldconfig: need absolute file name for configuration file when using -r
		sed -e 's,include ld.so.conf.d/\*.conf,include /etc/ld.so.conf.d/*.conf,' \
		    -i "$IAMROOT_ROOT/etc/ld.so.conf"
	fi

	exec "$real_path" "$@"
	;;
passwd|su)
	shift
	echo "Error:" "Command not handled:" "$argv0" "$@" >&2
	exit 1
	;;
bbsuid)
	for i in /bin/mount /bin/umount /bin/su /usr/bin/crontab /usr/bin/passwd /usr/bin/traceroute /usr/bin/traceroute6 /usr/bin/vlock
	do
		ln -sf /bin/bbsuid "$IAMROOT_ROOT$i"
	done
	;;
*)
	echo "Warning:" "Command not handled:" "$argv0" "$@" >&2
	;;
esac
