#!/bin/sed -f
#
# Copyright 2022-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

1,/^Dependencies resolved\./ {
	# Fedora XX - x86_64                               XX MB/s |  XX MB     XX:XX
	/^.\+\s-\s\S\+/s,^\(.\+\s-\s\S\+\)\s\+\S\+\s.B/s\s|\s\+\S\{1\,3\}\s.B\s\+\S\{2\,2\}:\S\{2\,2\}\s\+$,\1 XXX xB/s | XXX xB XX:XX,
	# Last metadata expiration check: H:mm:ss ago on Day XX Mon YYYY HH:mm:ss xM TZ.
	/^Last metadata expiration check: /s,:\s.*,: HH:mm:ss on Day XX Mon YYYY HH:mm:ss xM TZ.,
}

/^Dependencies resolved\./,/^Downloading Packages:/ {
	# ======================================================================================
	#  Package                         Arch    Version                         Repo     Size
	# ======================================================================================
	/^\s\+Package\(\s\+\S\+\)\{4,4\}/s,\s\+, ,g
	/^=\+/s,=\+,===============================,

	#  acl                             x86_64  2.3.1-2.fcXX                    fedora   XX k
	/^\(\s\+\S\+\)\{6,6\}/s,^\(\s\+\S\+\).*,\1,

	# elfutils-default-yama-scope
	#                            noarch 0.179-1.fcXX                     fedora  XX k
	/^\W\+\(\s\+\S\+\)\{5,5\}/d

	# Install  XXX Packages
	# Total download size: XXX M
	# Installed size: XXX M
	/[[:digit:]]\+\s\(Packages\|M\)$/s,[[:digit:]]\+\s\(Packages\|M\),XXX \1,

	# Installing Environment Groups:
	#  Minimal Install
	# Installing Groups:
	#  Core
	# (trailing spaces)
	s,\s\+$,,
}

/^Downloading Packages:/,/^Running transaction check/ {
	# (XXX/XXX): acl-2.3.1-2.fcXX.x86_64.rpm          X.X MB/s |  XX kB     XX:XX
	/^(\S\+\/\S\+): /d

	# --------------------------------------------------------------------------------
	/^-\+/s,-\+,----------------------------------,

	# Total                                           XXX MB/s | XXX MB     XX:XX
	/^Total/s,\s\+\S\+\s.B/s\s|\s\+\S\{1\,3\}\s.B\s\+\S\{2\,2\}:\S\{2\,2\}\s\+$, XXX xB/s | XXX xB XX:XX,

	# Fedora XX - x86_64                              XXX MB/s | XXX kB     XX:XX
	/^Fedora/s,\s\+\S\+\s.B/s\s|\s\+\S\{1\,3\}\s.B\s\+\S\{2\,2\}:\S\{2\,2\}\s\+$, XXX xB/s | XXX xB XX:XX,
}

/^Importing GPG key 0x[[:alnum:]]\+:/,/^Key imported successfully/ {
	# Fingerprint: XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX
	/ Fingerprint/s, [A-F0-9]\{4\,4\}, XXXX,g
}

/^Running transaction/,/^Installed:/ {
	#   Running scriptlet: filesystem-3.14-7.fcXX.x86_64                      XXX/XXX
	#   Installing       : filesystem-3.14-7.fcXX.x86_64                      XXX/XXX
	#   Verifying        : filesystem-3.14-7.fcXX.x86_64                      XXX/XXX
	/^\s\+\(\S\+\s*\)\{1,2\}: / {
		s,\([[:lower:][:upper:][:digit:]._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._~^-]\+\|[[:alnum:]._~^-]\+\)-\([[:alnum:].%{?}-]\+\)\.\([[:lower:][:digit:]]\+\.[[:alnum:]_]\+\|[[:alnum:]_]\+\),\1,
		s,\s\+[[:digit:]]\+\/[[:digit:]]\+\s\+$, XXX/XXX,
	}

	# /var/tmp/rpm-tmp.XXXXXX: line X: file: command not found
	/^\/var\/tmp\/rpm-tmp\./ {
		s,/rpm-tmp\.[[:alnum:]]\{6\,6\},/rpm-tmp.XXXXXX,
		s,line \([[:digit:]]\+\): ,line X: ,
	}

	#   Running scriptlet: openssh-server-8.5p1-2.fcXX.x86_64                 XXX/XXX
	# (2022-06-16 22:01:23:196328): [sss_cache] [confdb_get_enabled_domain_list] (0x0040): Failed to get [domains] from [sssd], error [2] (No such file or directory)
	# (2022-06-16 22:01:23:196440): [sss_cache] [init_domains] (0x0020): Could not initialize domains
	# (2022-06-16 22:01:23:225076): [sss_cache] [confdb_get_enabled_domain_list] (0x0040): Failed to get [domains] from [sssd], error [2] (No such file or directory)
	# (2022-06-16 22:01:23:225167): [sss_cache] [init_domains] (0x0020): Could not initialize domains
	s,^(\S\{4\,4\}-\S\{2\,2\}-\S\{2\,2\}\s\{1\,2\}\S\{1\,2\}:\S\{2\,2\}:\S\{2\,2\}:\S\{6\,6\}):\s,(YYYY-MM-DD hh:mm:ss:nnnnnn): ,

	#   Running scriptlet: systemd XXX/XXX
	# Creating group 'input' with GID XXX.
	# Creating group 'kvm' with GID XXX.
	# Creating group 'render' with GID XXX.
	# Creating group 'sgx' with GID XXX.
	# Creating group 'systemd-journal' with GID XXX.
	# Creating group 'systemd-network' with GID XXX.
	# Creating user 'systemd-network' (systemd Network Management) with UID XXX and GID XXX.
	# Creating group 'systemd-oom' with GID XXX.
	# Creating user 'systemd-oom' (systemd Userspace OOM Killer) with UID XXX and GID XXX.
	# Creating group 'systemd-resolve' with GID XXX.
	# Creating user 'systemd-resolve' (systemd Resolver) with UID XXX and GID XXX.
	# Failed to flush /etc/.#groupXXXXXXXXXXXXXXXX: No such file or directory
	/Failed to flush/s,/etc/\.#group[[:alnum:]]\{16\,16\}:,/etc/.#groupXXXXXXXXXXXXXXXX:,
}

/^Installed:/,$ {
	#   filesystem-3.14-7.fcXX.x86_64
	/^\s\s/ {
		s,\([[:lower:][:upper:][:digit:]._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._~^-]\+\|[[:alnum:]._~^-]\+\)-\([[:alnum:].%{?}-]\+\)\.\([[:lower:][:digit:]]\+\.[[:alnum:]_]\+\|[[:alnum:]_]\+\),\1,
		s,\s\+$,,
	}
}

# Creating group 'bin' with GID XXX.
# Creating user 'bin' (n/a) with UID XXX and GID XXX.
/^Creating \(user\|group\) '.*'/ {
	s,UID [[:digit:]]\+,UID XXX,
	s,GID [[:digit:]]\+,GID XXX,
}

# XXX blocks
/^[[:digit:]]\+ blocks/s,[[:digit:]]\+,XXX,
