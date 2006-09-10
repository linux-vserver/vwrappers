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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../wrapper.h"

#include "fstool.h"

const char *rcsid = "$Id$";

const char *optstring = "hvRcx:";

void usage(int rc)
{
	printf("Usage: chxid [-hvRc] -x <xid> <path>*\n"
	       "\n"
	       "Available Options:\n"
	       "  -h         Display this help text\n"
	       "  -v         Display version information\n"
	       "  -R         Recurse through directories\n"
	       "  -c         Cross filesystems\n"
	       "  -x <xid>   Context ID\n");
	exit(rc);
}

int handle_file(const char *fpath, const struct stat *sb,
                int tflag, struct FTW *ftwb)
{
	struct vx_iattr iattr = {
		.filename = fpath + ftwb->base,
		.xid      = fstool_args->xid,
		.flags    = IATTR_TAG,
		.mask     = IATTR_TAG,
	};
	
	/* check xid on the first run */
	if (ftwb->level == 0 && (fstool_args->xid == 1 || fstool_args->xid > 65535)) {
		err("invalid xid: %d", fstool_args->xid);
		return FTW_STOP;
	}
	
	/* unset xid tagging if xid == 0 */
	if (iattr.xid == 0)
		iattr.flags = 0;
	
	if (vx_set_iattr(&iattr) == -1) {
		perr("vx_set_iattr(%s)", fpath);
		errcnt++;
	}
	
	/* do not recurse (due to pre-order traversal, the first call of handle_file
	   is always the directory pointed to by command line arguments) */
	if (tflag == FTW_D && !fstool_args->recurse)
		return FTW_STOP;
	
	if (tflag == FTW_DNR)
		perr("could not read directory: %s", fpath);
	
	return FTW_CONTINUE;
}
