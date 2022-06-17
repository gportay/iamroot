#!/bin/bash
#
# Copyright 2021-2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

log() {
	level="$1"
	shift

	if [ ! -t 2 ] || [ "${NO_COLOR:-0}" -ne 0 ]
	then
		echo "$level:" "$@" >&2
		return
	fi

	echo -e "\e[31;1m$level:\e[0m" "$@" >&2
}

error() {
	log "Error" "$@"
}

warn() {
	log "Warning" "$@"
}

fixme() {
	log "FIXME" "$@"
}

path_resolution() {
	if [ "${1##*/}" != "$1" ]
	then
		echo "$PWD/${1#/*}"
		return
	fi

	echo "$IAMROOT_ROOT/${1#/*}"
}

path="$1"
inchroot_path="$(path_resolution "$path")"
argv0="${argv0:-$path}"
shift

case "${path##*/}" in
mount|umount)
	warn "Command is skipped:" "$argv0" "$@"
	exit 0
	;;
mountpoint)
	args=()
	for i in "$@"
	do
		if [ "${i:0:1}" = "-" ]
		then
			args+=("$i")
		else
			args+=("$IAMROOT_ROOT$i")
		fi
	done
	set -- "${args[@]}"

	warn "Command is rewritten to:" "$argv0" "$@"
	exec "$inchroot_path" "$@"
	;;
chfn|chkstat|pam-auth-update|update-ca-certificates|*.postinst)
	fixme "Command is skipped:" "$argv0" "$@"
	exit 0
	;;
ldd|busybox)
	exec "$inchroot_path" "$@"
	;;
ldconfig|ldconfig.real)
	if [ "${IAMROOT_ROOT:-/}" != / ]
	then
		set -- "$@" -r "$IAMROOT_ROOT"

		# Fixes: $IAMROOT_ROOT/usr/sbin/ldconfig: need absolute file name for configuration file when using -r
		sed -e 's,include ld.so.conf.d/\*.conf,include /etc/ld.so.conf.d/*.conf,' \
		    -i "$IAMROOT_ROOT/etc/ld.so.conf"
	fi

	exec "$inchroot_path" "$@"
	;;
gpasswd|passwd|su)
	error "Command not handled:" "$argv0" "$@"
	exit 1
	;;
bbsuid)
	for i in /bin/mount /bin/umount /bin/su /usr/bin/crontab /usr/bin/passwd /usr/bin/traceroute /usr/bin/traceroute6 /usr/bin/vlock
	do
		ln -sf /bin/bbsuid "$IAMROOT_ROOT$i"
	done
	exit 0
	;;
*)
	warn "host-running" "$argv0" "$@"
	exec "$path" "$@"
	;;
esac
