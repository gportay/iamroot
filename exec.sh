#!/bin/bash
#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

log() {
	if [[ "${IAMROOT_DEBUG:-0}" -lt 1 ]]
	then
		return
	fi

	echo "$@" >&2
}

case "${1##*/}" in
mount|umount)
	log "Warning:" "Command is skipped:" "$@"
	;;
*)
	log "Warning:" "Command not handled:" "$@"
	;;
esac
