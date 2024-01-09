#!/bin/sed -f
#
# Copyright 2023-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

# 123 packages will be downloaded
# 123 packages will be installed:
# 123 downloaded, 123 installed, 0 updated, 123 configured, 0 removed, 0 on hold.
/^[[:digit:]]\+\s/ {
	s,[[:digit:]]\+,XXX,g
}

#   base-files-0.143_3
# base-files-0.143_3.x86_64.xbps.sig: [512B 100%] 7576KB/s ETA: 00m00s
# base-files-0.143_3.x86_64.xbps.sig: 512B [avg rate: 7576KB/s]
# base-files-0.143_3.x86_64.xbps: [61KB 6%] 135MB/s ETA: 00m00s
# base-files-0.143_3.x86_64.xbps: 61KB [avg rate: 2049MB/s]
# base-files-0.143_3: verifying RSA signature...
# base-files: collecting files...
# base-files: unpacking ...
# base-files: configuring ...
# base-files: installed successfully.
/[[:space:]]*[[:lower:][:digit:]._+-]/ {
	s,\([[:lower:][:digit:]._+-]\+\)-\([[:alnum:]._+-]\+\)_\([[:alnum:]]\+\),\1,
	s,[[:digit:]]\+.B\(\|/s\),XXXxB\1,g
	s,[[:digit:]]\+%,XXX%,
	s,ETA: [[:digit:]]\{2\,2\}m[[:digit:]]\{2\,2\}s,ETA: MMmSSs,
	/ETA:/d
	/avg rate: /d
}

# dracut-install: convert_abs_rel(): from '/var/tmp/dracut.XXXXXX/initramfs/usr/bin' directory has no realpath: No such file or directory
# mknod: /var/tmp/dracut.XXXXXX/initramfs/dev/null: No such file or directory
# mknod: /var/tmp/dracut.XXXXXX/initramfs/dev/kmsg: No such file or directory
# mknod: /var/tmp/dracut.XXXXXX/initramfs/dev/console: No such file or directory
# mknod: /var/tmp/dracut.XXXXXX/initramfs/dev/random: No such file or directory
# mknod: /var/tmp/dracut.XXXXXX/initramfs/dev/urandom: No such file or directory
/\/var\/tmp\/dracut\./ {
	s,/dracut\.[[:alnum:]]\{6\,6\},/dracut.XXXXXX,
}

# .xbps-script-XXXXXX: line X: ./usr/libexec/xbps-triggers/kernel-hooks: No such file or directory
/^[[:alnum:]._+-]\+: line [[:digit:]]\+/ {
	s,-[[:alnum:]]\{6\,6\}:,-XXXXXX:,
	s,line [[:digit:]]\+:,line X:,
}
