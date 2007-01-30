// Copyright 2006 Benedikt Böhm <hollow@gentoo.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <vserver.h>
#include <sys/wait.h>
#include <lucid/str.h>
#include <lucid/open.h>

#include "wrapper.h"

static char *_vdir = NULL;

static
void lookup_vdir_vhiname(xid_t xid)
{
	char *vserverdir;
	vx_uname_t uname;

	uname.id = VHIN_CONTEXT;

	if (vx_uname_get(xid, &uname) == -1)
		return;

	/* util-vserver format */
	else if (uname.value[0] == '/')
		asprintf(&_vdir, "%s/vdir", uname.value);

	/* vcd format */
	else {
		vserverdir = strchr(uname.value, ':');
		*vserverdir++ = '\0';

		asprintf(&_vdir, "%s/%s", vserverdir, uname.value);
	}
}

static
void lookup_vdir_initpid(xid_t xid)
{
	int p[2], fd, status;
	pid_t pid;
	char procroot[PATH_MAX], buf[PATH_MAX];
	vx_info_t info;

	pipe(p);

	switch ((pid = fork())) {
	case -1:
		close(p[0]);
		close(p[1]);
		return;

	case 0:
		fd = open_read("/dev/null");

		dup2(fd,   0);
		dup2(p[1], 1);

		close(p[0]);
		close(p[1]);
		close(fd);

		if (vx_info(xid, &info) == -1)
			exit(EXIT_FAILURE);

		if (info.initpid < 2)
			exit(EXIT_FAILURE);

		snprintf(procroot, PATH_MAX, "/proc/%d/root", info.initpid);

		if (vx_migrate(1, NULL) == -1)
			exit(EXIT_FAILURE);

		bzero(buf, PATH_MAX);

		if (readlink(procroot, buf, PATH_MAX - 1) == -1)
			exit(EXIT_FAILURE);

		printf("%s", buf);

		exit(EXIT_SUCCESS);

	default:
		close(p[1]);
		str_readline(p[0], &_vdir);
		close(p[0]);

		if (waitpid(pid, &status, 0) == -1)
			return;

		if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
			return;

		if (WIFSIGNALED(status))
			return;
	}
}

static
void lookup_vdir_allpid(xid_t xid)
{
	/* this is ugly and will probably never be implemented */
}

char *lookup_vdir(xid_t xid, char *vdir, size_t len)
{
	lookup_vdir_vhiname(xid);

	if (!_vdir)
		lookup_vdir_initpid(xid);

	if (!_vdir)
		lookup_vdir_allpid(xid);

	if (_vdir) {
		strncpy(vdir, _vdir, len);
		return vdir;
	}

	return NULL;
}
