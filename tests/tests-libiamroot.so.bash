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

env-root() {
	( cd rootfs/ && env-host "PATH=$PWD/usr/bin" \
	                         "LD_LIBRARY_PATH=$PWD/usr/lib" \
	                         "IAMROOT_ROOT=$PWD" \
                                 "$@" )
}

env-host() {
	env "PATH=$PWD/rootfs/usr/bin" \
	    "LD_PRELOAD=$IAMROOT_ORIGIN/libiamroot.so" \
	    "IAMROOT_PATH_RESOLUTION_IGNORE=^/(proc|sys)/|.*\.gcda" \
	    "$@"
}

run "libiamroot.so: test path_resolution() looks up absolute path normally"
if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "$PWD/rootfs/usr/bin/test-path_resolution" | tee /dev/stderr | grep -q "^$PWD/rootfs/usr/bin/test-path_resolution$"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() looks up relative path normally"
if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "rootfs/usr/bin/test-path_resolution" | tee /dev/stderr | grep -q "^$PWD/rootfs/usr/bin/test-path_resolution$"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() follows relative symlinks in path component"
if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "$PWD/rootfs/bin/test-path_resolution" | tee /dev/stderr | grep -q "^$PWD/rootfs/usr/bin/test-path_resolution$"
then
	ok
else
	ko
fi
echo

if test -d /bin && ! test -L /bin
then
	run "libiamroot.so: test path_resolution() follows absolute symlinks in path component"
	if rm -f rootfs/tmp/bin && ln -sf /bin rootfs/tmp/bin &&
	   env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution "$PWD/rootfs/tmp/bin/test-path_resolution" | tee /dev/stderr | grep -q "^/bin/test-path_resolution$"
	then
		ok
	else
		ko
	fi
	echo
elif test -d /usr/bin && ! test -L /usr/bin
then
	run "libiamroot.so: test path_resolution() follows absolute symlinks in path component"
	if rm -f rootfs/tmp/bin && ln -sf /usr/bin rootfs/tmp/bin &&
	   env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution "$PWD/rootfs/tmp/bin/test-path_resolution" | tee /dev/stderr | grep -q "^/usr/bin/test-path_resolution$"
	then
		ok
	else
		ko
	fi
	echo
elif test -d /usr/local/bin && ! test -L /usr/local/bin
then
	run "libiamroot.so: test path_resolution() follows absolute symlinks in path component"
	if rm -f rootfs/tmp/bin && ln -sf /usr/local/bin rootfs/tmp/bin &&
	   env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution "$PWD/rootfs/tmp/bin/test-path_resolution" | tee /dev/stderr | grep -q "^/usr/local/bin/test-path_resolution$"
	then
		ok
	else
		ko
	fi
	echo
fi

run "libiamroot.so: test path_resolution() looks up absolute path normally (in-chroot)"
if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "/usr/bin/test-path_resolution" | tee /dev/stderr | grep -q "^$PWD/rootfs/usr/bin/test-path_resolution$"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() looks up relative path normally (in-chroot)"
if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "/usr/bin/test-path_resolution" | tee /dev/stderr | grep -q "^$PWD/rootfs/usr/bin/test-path_resolution$"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() follows relative symlinks in path component (in-chroot)"
if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "/bin/test-path_resolution" | tee /dev/stderr | grep -q "^$PWD/rootfs/usr/bin/test-path_resolution$"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() follows absolute symlinks in path component (in-chroot)"
if rm -f rootfs/tmp/bin && ln -sf /usr/bin rootfs/tmp/bin &&
   env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "/tmp/bin/test-path_resolution" | tee /dev/stderr | grep -q "^$PWD/rootfs/usr/bin/test-path_resolution$"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() follows the final symbolic link"
if rm -f rootfs/tmp/final-symbolic-link && ln -sf . rootfs/tmp/final-symbolic-link
   env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "$PWD/rootfs/tmp/final-symbolic-link" | tee /dev/stderr | grep -q "^$PWD/rootfs/tmp"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() does not follow the final symbolic link if AT_SYMLINK_FOLLOW"
if rm -f rootfs/tmp/final-symbolic-link && ln -sf . rootfs/tmp/final-symbolic-link
   env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "$PWD/rootfs/tmp/final-symbolic-link" 0x400 | tee /dev/stderr | grep -q "^$PWD/rootfs/tmp"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() does not follow the final symbolic link if AT_SYMLINK_NOFOLLOW"
if rm -f rootfs/tmp/final-symbolic-link && ln -sf . rootfs/tmp/final-symbolic-link
   env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "$PWD/rootfs/tmp/final-symbolic-link" 0x100 | tee /dev/stderr | grep -q "^$PWD/rootfs/tmp/final-symbolic-link"
then
	ok
else
	ko
fi
echo

run "libiamroot.so: test path_resolution() looks up inexistent file"
if rm -f rootfs/no-such-file-or-directory &&
   env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
            test-path_resolution "$PWD/rootfs/no-such-file-or-directory" | tee /dev/stderr | grep -q "^$PWD/rootfs/no-such-file-or-directory$"
then
	ok
else
	ko
fi
echo

if test -L /proc/1/cwd
then
	run "libiamroot.so: test path_resolution() looks up permission denied file"
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution "/proc/1/cwd/" | tee /dev/stderr | grep -q "^/proc/1/cwd/"
	then
		ok
	else
		ko
	fi
	echo
fi

if [[ "${XATTR:-0}" -eq 1 ]]
then
	run "libiamroot.so: test path_resolution() follows directory with user extended attribute added"
	if setfattr -n "user.iamroot.path-resolution" -v "/proc\0" rootfs/proc &&
	   env-host LD_LIBRARY_PATH="$PWD" test-path_resolution rootfs/proc | tee /dev/stderr | grep -q "^/proc$"
	then
		ok
	else
		ko
	fi
	echo
	
	run "libiamroot.so: test path_resolution() resolves directory with user extended attribute removed normally"
	if setfattr -x "user.iamroot.path-resolution" rootfs/proc &&
	   env-host LD_LIBRARY_PATH="$PWD" test-path_resolution rootfs/proc | tee /dev/stderr | grep -q "^$PWD/rootfs/proc$"
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
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution "this is a very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long path noexceeding PATH_MAX"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() does not resolve very long path exceeding PATH_MAX"
	if ! env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
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
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution / | tee /dev/stderr | grep -q "^/$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host virtual filesystem tmpfs(5) on mountpoint /run using host"
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /run/ | tee /dev/stderr | grep -q "^/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host directory /var/run using host"
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /var/run/ | tee /dev/stderr | grep -q "^/var/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem devtmpfs on mountpont /run internally (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /dev/ | tee /dev/stderr | grep -q "^$PWD/rootfs/dev$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem tmpfs(5) on mountpont /run internally (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /run | tee /dev/stderr | grep -q "^$PWD/rootfs/run$"
	then
		ok
	else
		ko
	fi
	echo


	run "libiamroot.so: test path_resolution() resolves directory /var/run internally (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
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
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution / | tee /dev/stderr | grep -q "^/$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host virtual filesystem tmpfs(5) on mountpoint /run using host"
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /run/ | tee /dev/stderr | grep -q "^/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host symlink /var/run using host"
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /var/run/ | tee /dev/stderr | grep -q "^/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves host virtual filesystem procfs(5) using host"
	if env-host "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /proc/ | tee /dev/stderr | grep -q "^/proc/$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem devtmpfs on mountpont /run internally (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /dev/ | tee /dev/stderr | grep -q "^$PWD/rootfs/dev$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem tmpfs(5) on mountpont /run internally (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /run | tee /dev/stderr | grep -q "^$PWD/rootfs/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves relative symlink /var/run internally (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /var/run | tee /dev/stderr | grep -q "^$PWD/rootfs/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves mountpoint /var/run internally (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution /var/run | tee /dev/stderr | grep -q "^$PWD/rootfs/run$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution() resolves virtual filesystem sysfs(5) using host (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
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
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution2 /proc/1/root 0x00 | tee /dev/stderr | grep -q "^/$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution2() does not resolve magic link if NOMAGICLINKS set (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution2 /proc/1/root 0x01 | tee /dev/stderr | grep -q "^/proc/1/root$"
	then
		ok
	else
		ko
	fi
	echo

	run "libiamroot.so: test path_resolution2() does not resolve anything if NOWALKALONG set (in-chroot)"
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
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
	if env-root "LD_LIBRARY_PATH=$PWD/rootfs/usr/lib:$PWD/rootfs/usr/local/lib" \
	            test-path_resolution2 /proc/1/root 0x03 | tee /dev/stderr | grep -q "^$PWD/rootfs/proc/1/root$"
	then
		ok
	else
		ko
	fi
	echo
fi
