#!/bin/sed -f
#
# Copyright 2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

1,$ {
	# XXX upgraded, XXX newly installed, XXX to remove and XXX not upgraded.
	/^[[:digit:]]\+\supgraded,/ {
		s,[[:digit:]]\+,XXX,g
	}

	# Need to get XXX.X xB of archives.
	# After this operation, XXX.X xB of additional disk space will be used.
	/^\(Need\|After\)/ {
		s,[[:digit:].]\+\s\([kM]\|\)B,XXX.X xB,g
	}

	# _FORTIFY_SOURCE requires compiling with optimization (-O) at /usr/lib/perl5/5.38/vendor_perl/features.ph line XXX.
	# Use of uninitialized value in concatenation (.) or string at /usr/share/perl5/Debconf/Config.pm line XXX.
	/line\s[[:digit:]]\+\./ {
		s,line\s\([[:digit:]]\+\)\.$,line XXX.,
	}

	# dpkg-deb: building package 'usr-is-merged' in '$ROOT/tmp/tmp.XXXXXXXXXX/usr-is-merged.deb'.
	/^dpkg-deb: / {
		s,/tmp\.[[:alnum:]]\{10\,10\},/tmp.XXXXXXXXXX,
	}

	# Fetched XXX xB in XXXmin XXXs (XXX xB/s)
	/^Fetched / {
		s,[[:digit:].]\{3\,4\}\s\([kM]\|\)B,XXX xB,g
		s,in\(\s[[:digit:]]\+\(min\|s\)\)\+,in XXXs,g
	}

	# Get:X http://deb.debian.org/debian bookworm InRelease [XXX kB]
	# Hit:X http://deb.debian.org/debian bookworm InRelease
	/^\(Get\|Hit\):/ {
		d
	}

	# /usr/bin/dpkg --status-fd 9 --no-triggers --unpack --auto-deconfigure $ROOT/amd64-mobian-trixie-rootfs/var/cache/apt/archives/base-files_13.2_amd64.deb
	/^\/usr\/bin\/dpkg / {
		s,--status-fd\s[[:digit:]]\+,--status-fd XXX,
		s,\([[:lower:][:digit:]+.-]\+\)_\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)_\([[:lower:][:digit:]-]\+\)\.deb,\1.deb,g
	}

	# Unpacking base-files (12.2) ...
	# Unpacking base-files (12.2) over (12.2) ...
	# Setting up base-files (12.2) ...
	# Unpacking libc6:amd64 (2.34-3) ...
	# Selecting previously unselected package libc6:amd64.
	# Processing triggers for libc-bin (2.33-7) ...
	/^\(Unpacking\|Selecting\|Setting up\|Processing triggers for\)/ {
		s,(\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\))\sover\s,,
		s,\([[:lower:][:digit:]+.-]\+\)\s(\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\))\s\.\.\.,\1 ...,
		s,\([[:lower:][:digit:]+.-]\+\):\([[:lower:][:digit:]]\+\),\1,
	}

	# Preparing to unpack .../base-files_12.2_amd64.deb ...
	/^Preparing to unpack/ {
		s,\.\.\./[[:digit:]]\+-,.../,
		s,\([[:lower:][:digit:]+.-]\+\)_\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)_\([[:lower:][:digit:]-]\+\)\.deb\s\.\.\.,\1.deb ...,
	}

	# dpkg: regarding .../archives/dpkg_1.21.1_amd64.deb containing dpkg, pre-dependency problem:
	#  dpkg pre-depends on libc6 (>= 2.15)
	#   libc6 is not installed.
	# dpkg: libc6:amd64: dependency problems, but configuring anyway as you requested:
	#  libc6:amd64 depends on libgcc-s1; however:
	#   Package libgcc-s1 is not installed.
	/^dpkg: /,/^$/ {
		s,\([[:lower:][:digit:]+.-]\+\)_\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)_\([[:lower:][:digit:]-]\+\)\.deb,\1.deb,
		s,\([[:lower:][:digit:]+.-]\+\)\s(>\?=\s\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)),\1,
		s,\([[:lower:][:digit:]+.-]\+\):\([[:lower:][:digit:]]\+\),\1,
	}

	# Local time is now:      Day Mon dd hh:mm:ss TZ YYYY.
	# Universal Time is now:  Day Mon dd hh:mm:ss TZ YYYY.
        /^\(Local time\|Universal Time\) is now: / {
		s,\(:\s\+\).*$,\1Day Mon dd hh:mm:ss TZ YYYY.,
	}

	# (Reading database ... XXXX files and directories currently installed.)
	/^(Reading database \.\.\./ {
		s,[[:digit:]]\+,XXX,g
	}

	# debconf is already the newest version (1.5.86).
	/^[[:lower:][:digit:]+.-]\+ is already the newest version/ {
		s,(\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)),(XXX),
	}

	# Warning: $ROOT/rootfs/tmp/mmdebstrap.apt.conf.XXXXXXXXXXXX: contains root directory '$ROOT/rootfs'
	# Warning: $ROOT/rootfs/var/cache/apt/srcpkgcache.bin.XXXXXX: contains root directory '$ROOT/rootfs'
	# Warning: $ROOT/rootfs/var/cache/apt/pkgcache.bin.XXXXXX: contains root directory '$ROOT/rootfs'
	/Warning: .\+\(\/tmp\/mmdebstrap\.apt\.conf\|\/var\/cache\/apt\/srcpkgcache\.bin\|\/var\/cache\/apt\/pkgcache\.bin\)\.[[:alnum:]_]\{6,12\}: contains root directory '.\+'/ {
		s,\.[[:alnum:]_]\{12\,12\}: contains root directory,.XXXXXXXXXXXX: contains root directory,
		s,\.[[:alnum:]_]\{6\,6\}: contains root directory,.XXXXXX: contains root directory,
	}
}

# Creating group 'bin' with GID XXX.
# Creating user 'bin' (n/a) with UID XXX and GID XXX.
/^Creating \(user\|group\) '.*'/ {
	s,UID [[:digit:]]\+,UID XXX,
	s,GID [[:digit:]]\+,GID XXX,
}

# Adding group `crontab' (GID XXX) ...
/^Adding group `.\+'/ {
	s,GID [[:digit:]]\+,GID XXX,
}

# I: success in XXX.XXXX seconds
$ {
	s,[[:digit:].]\+\sseconds,XXX.XXXX seconds,g
}
