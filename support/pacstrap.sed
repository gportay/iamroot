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

# :: [1;37mINFO [1;m Downloading mirrors from Manjaro
s,\x1b\[[[:digit:];]*m,,g

# (12/12) Configuring pacman-mirrors ...
# ::^[[1;37mINFO^[[1;m Downloading mirrors from Manjaro
# ::^[[1;37mINFO^[[1;m => Mirror pool: https://repo.manjaro.org/mirrors.json
# ::^[[1;37mINFO^[[1;m => Mirror status: https://repo.manjaro.org/status.json
# ::^[[1;37mINFO^[[1;m Using default mirror file
# ::^[[1;37mINFO^[[1;m Querying mirrors - This may take some time
#   ..... United_Kingdom : http://manjaro.mirrors.uk2.net/^M  ^[[1;32m0.149^[[1;m
#   ..... Canada         : https://mirror.0xem.ma/manjaro/^M  ^[[1;32m2.270^[[1;m
#   ..... Germany        : http://ftp.tu-chemnitz.de/pub/linux/manjaro/^M  ^[[1;32m0.300^[[1;m
#   ..... Netherlands    : https://ftp.nluug.nl/pub/os/Linux/distr/manjaro/^M  ^[[1;32m0.354^[[1;m
#   ..... Germany        : https://ftp.halifax.rwth-aachen.de/manjaro/^M  ^[[1;32m0.269^[[1;m
# ::^[[1;37mINFO^[[1;m Writing mirror list
# ::United_Kingdom  : http://manjaro.mirrors.uk2.net/stable^[[1;m
# ::Germany         : https://ftp.halifax.rwth-aachen.de/manjaro/stable^[[1;m
# ::Germany         : http://ftp.tu-chemnitz.de/pub/linux/manjaro/stable^[[1;m
# ::Netherlands     : https://ftp.nluug.nl/pub/os/Linux/distr/manjaro/stable^[[1;m
# ::Canada          : https://mirror.0xem.ma/manjaro/stable^[[1;m
# ::^[[1;37mINFO^[[1;m Mirror list generated and saved to: /etc/pacman.d/mirrorlist
# hint: use `pacman-mirrors` to generate and update your pacman mirrorlist.
/^([[:digit:] X]\+\/[[:digit:] X]\+) Configuring pacman-mirrors \.\.\./,/^hint: / {
	//p
	d
}
