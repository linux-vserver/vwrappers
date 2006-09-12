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

#ifndef _WRAPPER_H
#define _WRAPPER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <vserver.h>
#include <sys/wait.h>
#include <lucid/chroot.h>
#include <lucid/io.h>
#include <lucid/open.h>

static char vdir[PATH_MAX];

void err(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	
	vdprintf(STDERR_FILENO, msg, ap);
	dprintf(STDERR_FILENO, "\n");
	
	va_end(ap);
}

void perr(const char *msg, ...)
{
	char *errstr = strerror(errno);
	
	va_list ap;
	va_start(ap, msg);
	
	vdprintf(STDERR_FILENO, msg, ap);
	dprintf(STDERR_FILENO, ": %s\n", errstr);
	
	va_end(ap);
}

void die(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	
	vdprintf(STDERR_FILENO, msg, ap);
	dprintf(STDERR_FILENO, "\n");
	
	va_end(ap);
	exit(EXIT_FAILURE);
}

void pdie(const char *msg, ...)
{
	char *errstr = strerror(errno);
	
	va_list ap;
	va_start(ap, msg);
	
	vdprintf(STDERR_FILENO, msg, ap);
	dprintf(STDERR_FILENO, ": %s\n", errstr);
	
	va_end(ap);
	exit(EXIT_FAILURE);
}

void lookup_vdir(xid_t xid)
{
	int p[2], fd, status;
	pid_t pid;
	char procroot[PATH_MAX], buf[PATH_MAX], *data;
	struct vx_info info;
	
	pipe(p);
	
	switch ((pid = fork())) {
	case -1:
		pdie("fork");
	
	case 0:
		fd = open_read("/dev/null");
		
		dup2(fd,   0);
		dup2(p[1], 1);
		
		close(p[0]);
		close(p[1]);
		close(fd);
		
		if (vx_get_info(xid, &info) == -1)
			pdie("vx_get_info");
		
		/* TODO: recurse through process list and find one with xid */
		if (info.initpid < 2)
			die("invalid initpid");
		
		snprintf(procroot, PATH_MAX, "/proc/%d/root", info.initpid);
		
		if (vx_migrate(1, NULL) == -1)
			pdie("vx_migrate");
		
		bzero(buf, PATH_MAX);
		
		if (readlink(procroot, buf, PATH_MAX - 1) == -1)
			pdie("readlink");
		
		printf(buf);
		
		exit(EXIT_SUCCESS);
	
	default:
		close(p[1]);
		io_read_eof(p[0], &data);
		close(p[0]);
		
		bzero(vdir, PATH_MAX);
		strncpy(vdir, data, PATH_MAX - 1);
		free(data);
		
		if (waitpid(pid, &status, 0) == -1)
			pdie("waitpid");
		
		if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
			exit(EXIT_FAILURE);
		
		if (WIFSIGNALED(status)) {
			dprintf(STDERR_FILENO, "caught signal %d", WTERMSIG(status));
			exit(EXIT_FAILURE);
		}
	}
}

int default_wrapper(int argc, char **argv, char *proc, int needxid)
{
	xid_t xid;
	
	/* check for xid and shuffle arguments */
	if (argc > 2 && strcmp(argv[1], "-x") == 0) {
		xid = atoi(argv[2]);
		argv[2] = proc;
		argv = &argv[2];
		argc -= 2;
	}
	
	else {
		xid = 1;
		argv[0] = proc;
	}
	
	if (needxid && (xid < 2 || xid > 65535))
		die("xid must be between 2 and 65535");
	
	if (xid > 1) {
		lookup_vdir(xid);
		
		if (vx_enter_namespace(xid) == -1)
			pdie("vx_enter_namespace");
		
		if (chroot_secure_chdir(vdir, "/") == -1)
			pdie("chroot_secure_chdir");
		
		if (chroot(".") == -1)
			pdie("chroot");
	}
	
	if (vx_migrate(xid, NULL) == -1)
		pdie("vx_migrate");
	
	if (execvp(argv[0], argv) == -1)
		pdie("execvp");
	
	return EXIT_FAILURE;
}

#define DEFAULT_WRAPPER(PROC, NEEDXID) \
int main(int argc, char **argv) { \
	return default_wrapper(argc, argv, PROC, NEEDXID); \
}

#endif
