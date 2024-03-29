/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "iamroot.h"

extern int next_open(const char *, int, mode_t);

hidden ssize_t __interpreter_script_hashbang(const char *path, char *buf,
					     size_t bufsiz, off_t offset)
{
	ssize_t ret;
	char *d, *s;
	int fd;

	/*
	 * According to man execve(2):
	 *
	 * Interpreter scripts
	 *
	 * An interpreter script is a text file that has execute permission
	 * enabled and whose first line is of the form:
	 *
	 * #!interpreter [optional-arg]
	 *
	 * The interpreter must be a valid pathname for an executable file.
	 *
	 * If the pathname argument of execve() specifies an interpreter
	 * script, then interpreter will be invoked with the following
	 * arguments:
	 *
	 * interpreter [optional-arg] pathname arg...
	 *
	 * where pathname is the pathname of the file specified as the first
	 * argument of execve(), and arg... is the series of words pointed to
	 * by the argv argument of execve(), starting at argv[1]. Note that
	 * there is no way to get the argv[0] that was passed to the execve()
	 * call.
	 *
	 * For portable use, optional-arg should either be absent, or be
	 * specified as a single word (i.e., it should not contain white
	 * space); see NOTES below.
	 *
	 * Since Linux 2.6.28, the kernel permits the interpreter of a script
	 * to itself be a script. This permission is recursive, up to a limit
	 * of four recursions, so that the interpreter may be a script which is
	 * interpreted by a script, and so on.
	 */
	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = read(fd, &buf[offset], bufsiz-offset-1); /* NULL-terminated */
	if (ret == -1)
		goto close;
	buf[offset+ret] = 0; /* ensure NULL-terminated */

	/* Not an hashbang interpreter directive */
	if ((ret < 2) || (buf[offset+0] != '#') || (buf[offset+1] != '!')) {
		ret = __set_errno(ENOEXEC, -1);
		goto close;
	}

	s = buf+offset+2;
	d = buf+offset;

	/* skip leading blanks */
	while (isblank(*s))
		s++;
	/* copy interpreter */
	while (*s && *s != '\n' && !isblank(*s))
		*d++ = *s++;
	*d++ = 0;

	/*
	 * According to man execve(2):
	 *
	 * Interpreter scripts
	 *
	 * The semantics of the optional-arg argument of an interpreter script
	 * vary across implementations. On Linux, the entire string following
	 * the interpreter name is passed as a single argument to the
	 * interpreter, and this string can include white space. However,
	 * behavior differs on some other systems. Some systems use the first
	 * white space to terminate optional-arg. On some systems, an
	 * interpreter script can have multiple arguments, and white spaces in
	 * optional-arg are used to delimit the arguments.
	 */
	/* has an optional argument */
	if (isblank(*s)) {
		/* skip leading blanks */
		while (isblank(*s))
			s++;
		/* copy optional argument */
		while (*s && *s != '\n' && !isblank(*s))
			*d++ = *s++;
		*d++ = 0;
	}

	ret = d-buf-offset-1;

close:
	__close(fd);

	return ret;
}

hidden int __interpreter_script(const char *path, char * const argv[],
				char *buf, size_t bufsiz, off_t offset,
				char *interparg[])
{
	ssize_t siz;
	size_t len;
	int i = 0;
	(void)argv;

	/* Get the interpeter directive stored after the hashbang */
	siz = __interpreter_script_hashbang(path, buf, bufsiz, offset);
	if (siz < 1)
		return -1;

	/* Reset argv0 */
	interparg[i++] = &buf[offset]; /* hashbang as argv0 */

	/* Add optional argument */
	len = strnlen(&buf[offset], bufsiz-offset);
	if (len < (size_t)siz && buf[offset+len+1])
		interparg[i++] = &buf[offset+len+1];
	interparg[i++] = (char *)path; /* FIXME: original program path as first
					* positional argument */
	interparg[i] = NULL; /* ensure NULL-terminated */

	return i;
}
