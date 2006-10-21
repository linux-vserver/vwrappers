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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <vserver.h>
#include <lucid/chroot.h>
#include <lucid/log.h>

#include "wrapper.h"

extern const char *rcsid;

int default_wrapper(int argc, char **argv, char *proc, int needxid)
{
	int c;
	xid_t xid = 1;
	char vdir[PATH_MAX];
	
	log_options_t log_options = {
		.ident  = argv[0],
		.stderr = true,
	};
	
	log_init(&log_options);
	atexit(log_close);
	
	while (1) {
		c = getopt(argc, argv, "+hvx:");
		
		if (c == -1)
			break;
		
		switch (c) {
			case 'h':
				printf("Usage: %s [-x <xid>] [-- <args>]\n", argv[0]);
				exit(EXIT_SUCCESS);
			
			case 'v':
				printf("%s\n", rcsid); exit(EXIT_SUCCESS);
				break;
			
			case 'x':
				xid = atoi(optarg);
				break;
			
			default:
				printf("Usage: %s [-x <xid>] [-- <args>]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	
	argv[--optind] = proc;
	
	if (needxid && (xid < 2 || xid > 65535))
		log_error_and_die("invalid xid: %d", xid);
	
	if (xid > 1) {
		if (vx_get_info(xid, NULL) == -1)
			log_perror_and_die("vx_get_info");
			
		if (lookup_vdir(xid, vdir, PATH_MAX) == NULL)
			log_error_and_die("could not find vserver dir");
		
		if (ns_enter(xid) == -1)
			log_perror_and_die("vx_enter_namespace");
		
		if (chroot_secure_chdir(vdir, "/") == -1)
			log_perror_and_die("chroot_secure_chdir");
		
		if (chroot(".") == -1)
			log_perror_and_die("chroot");
	}
	
	if (vx_migrate(xid, NULL) == -1)
		log_perror_and_die("vx_migrate");
	
	if (execvp(argv[optind], argv+optind) == -1)
		log_perror_and_die("execvp");
	
	return EXIT_FAILURE;
}
