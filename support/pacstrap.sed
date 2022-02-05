#!/usr/bin/sed -f
#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

/^\(gpg:\|key\)\s/ {
	# gpg: revocation certificate stored as '/etc/pacman.d/gnupg.tmp/openpgp-revocs.d/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.rev'
	/\.rev'$/s,'\(.*\)/[0-9A-F]\{40\,40\}\.rev','\1XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.rev',

	# gpg: key XXXXXXXXXXXXXXXX marked as ultimately trusted
	# key "Pacman Keyring Master Key <pacman@localhost>" (XXXXXXXXXXXXXXXX)
	/key\s/s,[0-9A-F]\{16\,16\},XXXXXXXXXXXXXXXX,
}

/^:: Synchronizing package databases\.\.\./,/^:: / {
	#  core downloading...
	/^ .\+ downloading\.\.\.$/d

	# Packages (XXX) acl-2.3.1-2  ...  base-2-2
	/^Packages ([[:digit:]]\+)/ {
		s,([[:digit:]]\+),(XXX),
		s,\([[:lower:][:digit:]@._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._]\+\|[[:alnum:]._]\+\)-\([[:digit:].]\+\),\1,g
	}

	# Total Download Size:   XXX.XX MiB
	# Total Installed Size:  XXX.XX MiB
	/^Total \S\+ Size: / {
		s,[[:digit:].]\+,XXX,
		s,.iB,XiB,
	}
}

/^:: Retrieving packages\.\.\./,/^:: /{
	#  acl-2.3.1-2-x86_64 downloading...
	/^ .\+ downloading\.\.\.$/d
}

/^:: Running post-transaction hooks\.\.\./,$ {
	# (XXX/XXX) Creating system user accounts...
	/^([[:digit:] ]\+\/[[:digit:] ]\+)/s,^([[:digit:] ]\+\/[[:digit:] ]\+),(XXX/XXX),
}
