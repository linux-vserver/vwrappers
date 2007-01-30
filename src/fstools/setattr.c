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

#include <stdlib.h>

#define _LUCID_PRINTF_MACROS
#include <lucid/log.h>
#include <lucid/printf.h>

#include "fstool.h"

const char *rcsid = "$Id$";

const char *optstring = "hvRcf:";

void usage(int rc)
{
	printf("Usage: setattr [-hvRc] -f <list> <path>\n"
	       "\n"
	       "Available Options:\n"
	       "  -h         Display this help text\n"
	       "  -v         Display version information\n"
	       "  -R         Recurse through directories\n"
	       "  -c         Cross filesystem mounts\n"
	       "  -f <list>  Attribute list (see 'vlist -ix-attr')\n");
	exit(rc);
}

int handle_file(const char *fpath, const struct stat *sb,
                int tflag, struct FTW *ftwb)
{
	ix_attr_t attr = {
		.filename = fpath + ftwb->base,
		.xid      = fstool_args->xid,
		.flags    = fstool_args->flags,
		.mask     = fstool_args->mask,
	};

	if (ix_attr_set(&attr) == -1) {
		log_perror("ix_set_attr(%s)", fpath);
		errcnt++;
	}

	/* do not recurse (due to pre-order traversal, the first call of handle_file
	   is always the directory pointed to by command line arguments) */
	if (tflag == FTW_D && !fstool_args->recurse)
		return FTW_STOP;

	/* if the top-level directory can't be read it will stop anyway, any other
	   directory that can't be read appears only if recurse is enabled */
	if (tflag == FTW_DNR) {
		log_error("could not read directory: %s", fpath);
		errcnt++;
	}

	return FTW_CONTINUE;
}
