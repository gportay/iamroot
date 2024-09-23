#!/bin/bash
#
# Copyright 2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

set -e
set -o pipefail

run() {
	lineno="${BASH_LINENO[0]}"
	test="$*"
	if [ -t 1 ]
	then
		echo -e "\e[1mRunning $test...\e[0m"
	else
		echo "Running $test..."
	fi
}

ok() {
	ok=$((ok+1))
	if [ -t 1 ]
	then
		echo -e "\e[1m$test: \e[32m[OK]\e[0m"
	else
		echo "$test: [OK]"
	fi
}

ko() {
	ko=$((ko+1))
	if [ -t 1 ]
	then
		echo -e "\e[1m$test: \e[31m[KO]\e[0m"
	else
		echo "$test: [KO]"
	fi
	if [ -t 2 ]
	then
		reports+=("$test at line \e[1m$lineno \e[31mhas failed\e[0m!")
	else
		reports+=("$test at line $lineno has failed!")
	fi
	if [[ $EXIT_ON_ERROR ]]
	then
		exit 1
	fi
}

fix() {
	fix=$((fix+1))
	if [ -t 1 ]
	then
		echo -e "\e[1m$test: \e[34m[FIX]\e[0m"
	else
		echo "$test: [FIX]"
	fi
	if [ -t 2 ]
	then
		reports+=("$test at line \e[1m$lineno is \e[34mfixed\e[0m!")
	else
		reports+=("$test at line $lineno is fixed!")
	fi
}

bug() {
	bug=$((bug+1))
	if [ -t 1 ]
	then
		echo -e "\e[1m$test: \e[33m[BUG]\e[0m"
	else
		echo "$test: [BUG]"
	fi
	if [ -t 2 ]
	then
		reports+=("$test at line \e[1m$lineno is \e[33mbugged\e[0m!")
	else
		reports+=("$test at line $lineno is bugged!")
	fi
}

result() {
	exitcode="$?"
	trap - 0

	if [ -t 2 ]
	then
		echo -e "\e[1mTest report:\e[0m"
	else
		echo "Test report:"
	fi
	for report in "${reports[@]}"
	do
		if [ -t 2 ]
		then
			echo -e "$report" >&2
		else
			echo "$report" >&2
		fi
	done

	if [[ $ok ]]
	then
		if [ -t 1 ]
		then
			echo -e "\e[1m\e[32m$ok test(s) succeed!\e[0m"
		else
			echo "$ok test(s) succeed!"
		fi
	fi

	if [[ $fix ]]
	then
		if [ -t 2 ]
		then
			echo -e "\e[1m\e[34m$fix test(s) fixed!\e[0m" >&2
		else
			echo "$fix test(s) fixed!" >&2
		fi
	fi

	if [[ $bug ]]
	then
		if [ -t 2 ]
		then
			echo -e "\e[1mWarning: \e[33m$bug test(s) bug!\e[0m" >&2
		else
			echo "Warning: $bug test(s) bug!" >&2
		fi
	fi

	if [[ $ko ]]
	then
		if [ -t 2 ]
		then
			echo -e "\e[1mError: \e[31m$ko test(s) failed!\e[0m" >&2
		else
			echo "Error: $ko test(s) failed!" >&2
		fi
	fi

	if [[ $exitcode -ne 0 ]] && [[ $ko ]]
	then
		if [ -t 2 ]
		then
			echo -e "\e[1;31mExited!\e[0m" >&2
		else
			echo "Exited!" >&2
		fi
	elif [[ $exitcode -eq 0 ]] && [[ $ko ]]
	then
		exit 1
	fi

	exit "$exitcode"
}

PATH="$IAMROOT_ORIGIN:$PWD:$PATH"
trap result 0 SIGINT

export -n NO_COLOR
export -n ISHLVL
export -n ISH_RC_FILE
export -n ISH_PROFILE_FILE
export -n IAMROOT_USER
export -n IAMROOT_ROOT
export -n IAMROOT_DEFLIB
export -n IAMROOT_EXEC
# Ignore: export -n IAMROOT_LIB
export -n IAMROOT_DEBUG
export -n IAMROOT_DEBUG_FD
export -n IAMROOT_DEBUG_IGNORE
export -n IAMROOT_EXEC_IGNORE
export -n IAMROOT_PATH_RESOLUTION_IGNORE

OS="$(uname -o 2>/dev/null || uname -s 2>/dev/null)"
XATTR="$(command -v getfattr >/dev/null 2>&1 && echo 1)"
PROCFS="$(test -d /proc/1 && ! test -r /proc/1/root && echo 1)"

env-in-chroot() {
	( cd rootfs/ && env-root "PATH=$PWD/usr/bin" \
	                         "LD_LIBRARY_PATH=$PWD/usr/lib" \
	                         "IAMROOT_ROOT=$PWD" \
                                 "$@" )
}

env-root() {
	env "PATH=$PWD/rootfs/usr/bin" \
	    "LD_PRELOAD=$IAMROOT_ORIGIN/libiamroot.so" \
	    "IAMROOT_PATH_RESOLUTION_IGNORE=^/(proc|sys)/|.*\.gcda" \
	    "$@"
}

if [[ "${XATTR:-0}" -eq 1 ]]
then
	run "libiamroot.so: test path_resolution() follows directory with user extended attribute added"
	if setfattr -n "user.iamroot.path-resolution" -v "/proc\0" rootfs/proc &&
	   env-root LD_LIBRARY_PATH="$PWD" test-path_resolution rootfs/proc | tee /dev/stderr | grep -q "^/proc$"
	then
		ok
	else
		ko
	fi
	echo
	
	run "libiamroot.so: test path_resolution() resolves directory with user extended attribute removed normally"
	if setfattr -x "user.iamroot.path-resolution" rootfs/proc &&
	   env-root LD_LIBRARY_PATH="$PWD" test-path_resolution rootfs/proc | tee /dev/stderr | grep -q "^$PWD/rootfs/proc$"
	then
		ok
	else
		ko
	fi
	echo
fi

if [[ "${OS:-0}" == "GNU/Linux" ]]
then
	run "libiamroot.so: test path_resolution() resolves very long path noexceeding PATH_MAX"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution "this is a very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long path noexceeding PATH_MAX"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() does not resolve very long path exceeding PATH_MAX"
	if ! env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	     test-path_resolution "this is a very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long path exceeding PATH_MAX"
	then
		ok
	else
		ko
	fi
	echo

fi

if [[ "${OS:-0}" == "FreeBSD" ]]
then
	run "libiamroot.so: test path_resolution() resolves host root directory using host"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution / | tee /dev/stderr | grep -q "^/$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host virtual filesystem tmpfs(5) on mountpoint /run using host"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /run/ | tee /dev/stderr | grep -q "^/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host directory /var/run using host"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /var/run/ | tee /dev/stderr | grep -q "^/var/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem devtmpfs on mountpont /run internally (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution /dev/ | tee /dev/stderr | grep -q "^$PWD/rootfs/dev$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem tmpfs(5) on mountpont /run internally (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution /run | tee /dev/stderr | grep -q "^$PWD/rootfs/run$"
	then
		ok
	else
		ko
	fi
	echo


	run "libiamroot.so: test path_resolution() resolves directory /var/run internally (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution /var/run | tee /dev/stderr | grep -q "^$PWD/rootfs/var/run$"
	then
		ok
	else
		ko
	fi
	echo

fi

if [[ "${OS:-0}" == "GNU/Linux" ]]
then
	run "libiamroot.so: test path_resolution() resolves host root directory using host"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution / | tee /dev/stderr | grep -q "^/$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host virtual filesystem tmpfs(5) on mountpoint /run using host"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /run/ | tee /dev/stderr | grep -q "^/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host symlink /var/run using host"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /var/run/ | tee /dev/stderr | grep -q "^/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host virtual filesystem procfs(5) using host"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /proc/ | tee /dev/stderr | grep -q "^/proc/$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem devtmpfs on mountpont /run internally (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution /dev/ | tee /dev/stderr | grep -q "^$PWD/rootfs/dev$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem tmpfs(5) on mountpont /run internally (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution /run | tee /dev/stderr | grep -q "^$PWD/rootfs/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves relative symlink /var/run internally (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution /var/run | tee /dev/stderr | grep -q "^$PWD/rootfs/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves mountpoint /var/run internally (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution /var/run | tee /dev/stderr | grep -q "^$PWD/rootfs/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem sysfs(5) using host (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution /sys/ | tee /dev/stderr | grep -q "^/sys/$"
	then
		ok
	else
		ko
	fi
	echo
fi

if [[ "${PROCFS:-0}" -eq 1 ]]
then
	run "libiamroot.so: test path_resolution2() resolves magic links if no PR flag set (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution2 /proc/1/root 0x00 | tee /dev/stderr | grep -q "^/$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution2() does not resolve magic link if NOMAGICLINKS set (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution2 /proc/1/root 0x01 | tee /dev/stderr | grep -q "^/proc/1/root$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution2() does not resolve anything if NOWALKALONG set (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution2 /proc/1/root 0x03 | tee /dev/stderr | grep -q "^$PWD/rootfs/proc/1/root$"
	then
		ok
	else
		ko
	fi
	echo
fi

if [[ "${PROCFS:-0}" -eq 1 ]] && [[ "${XATTR:-0}" -eq 1 ]]
then
	run "libiamroot.so: test path_resolution2() resolves virtual filesystem proc(5) internally if NOWALKALONG set (in-chroot)"
	if env-in-chroot "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	                 test-path_resolution2 /proc/1/root 0x03 | tee /dev/stderr | grep -q "^$PWD/rootfs/proc/1/root$"
	then
		ok
	else
		ko
	fi
	echo
fi
