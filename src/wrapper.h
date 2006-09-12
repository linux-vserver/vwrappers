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
#include <lucid/log.h>
#include <lucid/open.h>

static char vdir[PATH_MAX];

static
void lookup_vdir(xid_t xid)
{
	int p[2], fd, status;
	pid_t pid;
	char procroot[PATH_MAX], buf[PATH_MAX], *data;
	struct vx_info info;
	
	pipe(p);
	
	switch ((pid = fork())) {
	case -1:
		log_error_and_die("fork: %m");
	
	case 0:
		fd = open_read("/dev/null");
		
		dup2(fd,   0);
		dup2(p[1], 1);
		
		close(p[0]);
		close(p[1]);
		close(fd);
		
		if (vx_get_info(xid, &info) == -1)
			log_error_and_die("vx_get_info: %m");
		
		/* TODO: recurse through process list and find one with xid */
		if (info.initpid < 2)
			log_error_and_die("invalid initpid");
		
		snprintf(procroot, PATH_MAX, "/proc/%d/root", info.initpid);
		
		if (vx_migrate(1, NULL) == -1)
			log_error_and_die("vx_migrate: %m");
		
		bzero(buf, PATH_MAX);
		
		if (readlink(procroot, buf, PATH_MAX - 1) == -1)
			log_error_and_die("readlink: %m");
		
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
			log_error_and_die("waitpid: %m");
		
		if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
			exit(EXIT_FAILURE);
		
		if (WIFSIGNALED(status))
			log_error_and_die("caught signal %d", WTERMSIG(status));
	}
}

static
int default_wrapper(int argc, char **argv, char *proc, int needxid)
{
	xid_t xid;
	
	log_options_t log_options = {
		.ident  = argv[0],
		.file   = false,
		.stderr = true,
		.syslog = false,
	};
	
	log_init(&log_options);
	
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
		log_error_and_die("xid must be between 2 and 65535");
	
	if (xid > 1) {
		lookup_vdir(xid);
		
		if (vx_enter_namespace(xid) == -1)
			log_error_and_die("vx_enter_namespace: %m");
		
		if (chroot_secure_chdir(vdir, "/") == -1)
			log_error_and_die("chroot_secure_chdir: %m");
		
		if (chroot(".") == -1)
			log_error_and_die("chroot: %m");
	}
	
	if (vx_migrate(xid, NULL) == -1)
		log_error_and_die("vx_migrate: %m");
	
	if (execvp(argv[0], argv) == -1)
		log_error_and_die("execvp: %m");
	
	return EXIT_FAILURE;
}

#define DEFAULT_WRAPPER(PROC, NEEDXID) \
int main(int argc, char **argv) { \
	return default_wrapper(argc, argv, PROC, NEEDXID); \
}

#endif
