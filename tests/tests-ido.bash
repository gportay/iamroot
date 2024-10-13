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

run "ido: test without argument"
if ! ido
then
	ok
else
	ko
fi
echo

run "ido: test environment is not preserved"
if ! ( export FOO=foo && ido env | tee /dev/stderr | grep -q "^FOO=foo$" )
then
	ok
else
	ko
fi
echo

run "ido: test option -C=101 do not closes file descriptors < 100"
if ( exec 100>&2 && ido -C=101 readlink /proc/self/fd/100 )
then
	ok
else
	ko
fi
echo

run "ido: test option --close-from=100 closes file descriptor >= 100"
if ( exec 100>&2 && ! ido --close-from=100 readlink /proc/self/fd/100 )
then
	ok
else
	ko
fi
echo

run "ido: test option -D=/tmp changes working directory"
if ido -D=/tmp pwd | tee /dev/stderr | grep -q "^/tmp$"
then
	ok
else
	ko
fi
echo

run "ido: test option --chdir=/dev/null fails to change working directory"
if ! ido --chdir=/dev/null pwd
then
	ok
else
	ko
fi
echo

run "ido: test option -E preserve user environment when running command"
if ( export FOO=foo && ido -E env | tee /dev/stderr | grep -q "^FOO=foo$" )
then
	ok
else
	ko
fi
echo

run "ido: test option --preserve-env=FOO,BAR preserves specific environment variables FOO and BAR"
if ( export FOO=foo; export BAR=bar && ido --preserve-env=FOO,BAR env | tee /dev/stderr | grep -q "^FOO=foo$" &&
     export FOO=foo; export BAR=bar && ido --preserve-env=FOO,BAR env | tee /dev/stderr | grep -q "^BAR=bar$")
then
	ok
else
	ko
fi
echo

run "ido: test options --preserve-env=FOO,BAZ --preserve-env=BAZ preserves specific environment variables FOO, BAR and BAZ"
if ( export FOO=foo; export BAR=bar; export BAZ=baz && ido --preserve-env=FOO,BAR --preserve-env=BAZ env | tee /dev/stderr | grep -q "^FOO=foo$" &&
     export FOO=foo; export BAR=bar; export BAZ=baz && ido --preserve-env=FOO,BAR --preserve-env=BAZ env | tee /dev/stderr | grep -q "^BAR=bar$" &&
     export FOO=foo; export BAR=bar; export BAZ=baz && ido --preserve-env=FOO,BAR --preserve-env=BAZ env | tee /dev/stderr | grep -q "^BAZ=baz$" )
then
	ok
else
	ko
fi
echo

run "ido: test option -g=root runs command as group name root"
if ido -g=root id -gn | tee /dev/stderr | grep -q "^root$"
then
	ok
else
	ko
fi
echo

run "ido: test option -g=\#0 runs command as group ID 0"
if ido -g=0 id -g | tee /dev/stderr | grep -q "^0$"
then
	ok
else
	ko
fi
echo

run "ido: test option --group=\#${GROUPS[0]} runs command as group ID ${GROUPS[0]}"
if ido --group="${GROUPS[0]}" id -g | tee /dev/stderr | grep -q "^${GROUPS[0]}$"
then
	ok
else
	ko
fi
echo

run "ido: test option -H sets HOME variable to root's home directory"
if ido -H env | tee /dev/stderr | grep -q "^HOME=/root$"
then
	ok
else
	ko
fi
echo

run "ido: test option --set-home sets HOME variable to target user's home directory"
if ido -u="$USER" --set-home env | tee /dev/stderr | grep -q "^HOME=$HOME$"
then
	ok
else
	ko
fi
echo

run "ido: test option -h displays help message and exit"
if ido -h | grep "^usage:"
then
	ok
else
	ko
fi
echo

run "ido: test option --help displays help message and exit"
if ido --help | grep "^usage:"
then
	ok
else
	ko
fi
echo

run "ido: test option -i runs login shell as root user"
if ( export SHELL=/bin/nologin && ido -i <<<whoami | tee /dev/stderr | grep -q -E "^root$" )
then
	ok
else
	ko
fi
echo

run "ido: test option --login runs login shell as root user with the specified command"
if ( export SHELL=/bin/nologin && ido --login whoami | tee /dev/stderr | grep -q -E "^root$" )
then
	ok
else
	ko
fi
echo

run "ido: test option -P preserves group vector 0 ${GROUPS[*]}"
if ( export SHELL=bash && ido -P -s <<<'echo "GROUPS=${GROUPS[*]}"' | tee /dev/stderr | grep -q "^GROUPS=0 ${GROUPS[*]}$" )
then
	ok
else
	ko
fi
echo

run "ido: test option --preserve-groups preserves group vector ${GROUPS[*]}"
if ( export SHELL=/bin/bash && ido --user="$USER" --preserve-groups --shell <<<'echo "GROUPS=${GROUPS[*]}"' | tee /dev/stderr | grep -q "GROUPS=${GROUPS[*]}" )
then
	ok
else
	ko
fi
echo

run "ido: test option -R=$PWD/rootfs changes root directory"
if ido -R="$PWD/rootfs" env | tee /dev/stderr | grep -q "^IAMROOT_ROOT=$PWD/rootfs$"
then
	ok
else
	ko
fi
echo

run "ido: test option --chroot=/dev/null fails to change root directory"
if ! ido --chroot=/dev/null env
then
	ok
else
	ko
fi
echo

run "ido: test option -s runs /bin/sh"
if ( export SHELL=/bin/sh; ido -s <<<env | tee /dev/stderr | grep -q '^SHELL=/bin/sh$' )
then
	ok
else
	ko
fi
echo

run "ido: test option --shell runs /bin/bash with the specified command"
if ( export SHELL=/bin/bash; ido --shell env | tee /dev/stderr | grep -q '^SHELL=/bin/bash$' )
then
	ok
else
	ko
fi
echo

run "ido: test option -T terminates command after the specfified time limit"
if ( ido -T=0.1 sleep 5; test "$?" -eq 124 )
then
	ok
else
	ko
fi
echo

run "ido: test option --command-timeout does not terminate command after the specfified time limit"
if ( ido --command-timeout=5 sleep 1; test "$?" -eq 0 )
then
	ok
else
	ko
fi
echo

run "ido: test option -u=root runs command as user name root"
if ido -u=root id -un | tee /dev/stderr | grep -q "^root$"
then
	ok
else
	ko
fi
echo

run "ido: test option -u=\#0 runs command as user ID 0"
if ido -u=0 id -u | tee /dev/stderr | grep -q "^0$"
then
	ok
else
	ko
fi
echo

run "ido: test option --user=$USER runs command as user name $USER"
if ido --user="$USER" id -un | tee /dev/stderr | grep -q "^$USER$"
then
	ok
else
	ko
fi
echo

run "ido: test option --user=\#$UID runs command as user ID $UID"
if ido --user="#$UID" id -u | tee /dev/stderr | grep -q "^$UID$"
then
	ok
else
	ko
fi
echo

run "ido: test option -V displays version information and exit"
if ido -V | tee /dev/stderr | grep -q -E '^([0-9a-zA-Z]+)(\.[0-9a-zA-Z]+)*$'
then
	ok
else
	ko
fi
echo

run "ido: test option --version displays version information and exit"
if ido --version | tee /dev/stderr | grep -q -E '^([0-9a-zA-Z]+)(\.[0-9a-zA-Z]+)*$'
then
	ok
else
	ko
fi
echo

run "ido: test option --secure-path /usr/local/bin:/usr/bin:/usr/bin sets path used for every command run from ido"
if bash -x ido --secure-path /usr/local/bin:/usr/bin:/usr/bin env | grep "^PATH=/usr/local/bin:/usr/bin:/usr/bin$"
then
	ok
else
	ko
fi
echo

run "ido: test option --multiarch uses multiarch library path in chroot"
if ido --multiarch env | grep "^IAMROOT_MULTIARCH=1$"
then
	ok
else
	ko
fi
echo

run "ido: test option --libdir set /lib:/usr/lib as default library path in chroot"
if ido --libdir env | grep "^IAMROOT_DEFLIB=/lib:/usr/lib$"
then
	ok
else
	ko
fi
echo

run "ido: test option --deflib=/usr/local/lib:/lib:/usr/lib sets default library path to use in chroot"
if ido --deflib=/usr/local/lib:/lib:/usr/lib env | grep "^IAMROOT_DEFLIB=/usr/local/lib:/lib:/usr/lib$"
then
	ok
else
	ko
fi
echo

run "ido: test option --exec=/dev/null sets the absolute path to the iamroot exec script to use"
if ido --exec=/dev/null env | grep "^IAMROOT_EXEC=/dev/null$"
then
	ok
else
	ko
fi
echo

run "ido: test option --library=/dev/null sets the absolute path to the iamroot library to use"
if ido --library /dev/null env | grep "^IAMROOT_LIB=/dev/null$"
then
	ok
else
	ko
fi
echo

run "ido: test option --no-color turns off color"
if ido --no-color env | grep "^NO_COLOR=1$"
then
	ok
else
	ko
fi
echo

run "ido: test option --debug turns of debug mode"
if ido --debug env | grep "^IAMROOT_DEBUG=1$"
then
	ok
else
	ko
fi
echo

run "ido: test option --debug --debug turns of debug mode level 2"
if ido --debug --debug env | grep "^IAMROOT_DEBUG=2$"
then
	ok
else
	ko
fi
echo

run "ido: test option --debug-fd=100 sets debug fd"
if ( exec 100>&2 && ido --debug-fd=100 env | grep "^IAMROOT_DEBUG_FD=100$" )
then
	ok
else
	ko
fi
echo

run "ido: test option --debug-fd=100 sets and duplicates debug fd"
if ido --debug-fd=100 env | grep "^IAMROOT_DEBUG_FD=100$" &&
   ido --debug-fd=100 readlink /proc/self/fd/100
then
	ok
else
	ko
fi
echo

run "ido: test option --debug-ignore=^(chroot|chdir|fchdir)$ sets regular expression of path to ignore for path resolution in chroot."
if ido --debug-ignore="^(chroot|chdir|fchdir)$" env | grep "^IAMROOT_DEBUG_IGNORE=^(chroot|chdir|fchdir)\\$\$"
then
	ok
else
	ko
fi
echo

run "ido: test option --exec-ignore=^(ldd|ldconfig)$ sets regular expression of path to ignore for path resolution in chroot."
if ido --exec-ignore="^(ldd|ldconfig)$" env | grep "^IAMROOT_EXEC_IGNORE=^(ldd|ldconfig)\\$\$"
then
	ok
else
	ko
fi
echo

run "ido: test option --path-resolution-ignore=^(/proc/|/sys/|/dev/)$ sets regular expression of path to ignore for path resolution in chroot."
if ido --path-resolution-ignore="^(/proc/|/sys/|/dev/)$" env | grep "^IAMROOT_PATH_RESOLUTION_IGNORE=^(/proc/|/sys/|/dev/)\\$\$"
then
	ok
else
	ko
fi
echo

run "ido: test both options -i -s"
if ! ido -i -s
then
	ok
else
	ko
fi
echo

run "ido: test both options -i -E"
if ! ido -i -E
then
	ok
else
	ko
fi
echo

run "ido: test environment sets LOGNAME to the login name of the target user"
if ido -s <<<'echo "LOGNAME=$LOGNAME"' | tee /dev/stderr | grep -q "^LOGNAME=root$"
then
	ok
else
	ko
fi
echo

run "ido: test environment sets IDO_COMMAND to the command run by ido"
if ( export SHELL=/bin/sh && ido -s <<<'echo "IDO_COMMAND=$IDO_COMMAND"' | tee /dev/stderr | grep -q "^IDO_COMMAND=/bin/sh$" )
then
	ok
else
	ko
fi
echo

run "ido: test environment sets IDO_COMMAND to the command run by ido"
if ( ido -i <<<'echo "IDO_COMMAND=$IDO_COMMAND"' | tee /dev/stderr | grep -q -E "^IDO_COMMAND=[A-Za-z0-9/]+$" )
then
	ok
else
	ko
fi
echo

run "ido: test environment sets IDO_COMMAND to the command run by ido, including args"
if ( export SHELL=/bin/sh && ido -s env | tee /dev/stderr | grep -q "^IDO_COMMAND=/bin/sh -c 'env'$" )
then
	ok
else
	ko
fi
echo

run "ido: test environment sets IDO_GID to the group-ID of the user who invoked ido"
if ido -s <<<'echo "IDO_GID=$IDO_GID"' | tee /dev/stderr | grep -q "^IDO_GID=${GROUPS[0]}$"
then
	ok
else
	ko
fi
echo

run "ido: test environment sets IDO_UID to the user-ID of the user who invoked ido"
if ido -s <<<'echo "IDO_UID=$IDO_UID"' | tee /dev/stderr | grep -q "^IDO_UID=$UID$"
then
	ok
else
	ko
fi
echo

run "ido: test environment sets IDO_USER to the login name of the user who invoked ido"
if ido -s <<<'echo "IDO_USER=$IDO_USER"' | tee /dev/stderr | grep -q "^IDO_USER=$USER$"
then
	ok
else
	ko
fi
echo

run "ido: test environment sets USER to the same value as LOGNAME"
if ido -s <<<'echo "$USER=$LOGNAME"' | tee /dev/stderr | grep -q "^root=root$"
then
	ok
else
	ko
fi
echo
