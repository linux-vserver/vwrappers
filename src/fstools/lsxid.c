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

const char *optstring = "hvRcd";

void usage(int rc)
{
	printf("Usage: lsxid [-hvRcd] <path>*\n"
	       "\n"
	       "Available Options:\n"
	       "  -h         Display this help text\n"
	       "  -v         Display version information\n"
	       "  -R         Recurse through directories\n"
	       "  -c         Cross filesystems\n"
	       "  -d         Display directory instead of contents\n");
	exit(rc);
}

int handle_file(const char *fpath, const struct stat *sb,
                int tflag, struct FTW *ftwb)
{
	ix_attr_t attr;

	attr.filename = fpath + ftwb->base;

	if (tflag == FTW_DNR) {
		log_error("could not read directory: %s", fpath);
		errcnt++;
		return FTW_CONTINUE;
	}

	if (ix_attr_get(&attr) == -1) {
		log_error("ix_attr_get(%s): %m", fpath);
		errcnt++;
		return FTW_CONTINUE;
	}

	if (attr.mask & IATTR_TAG)
		printf("%5d %s%s", attr.xid, fpath, tflag == FTW_D ? "/\n" : "\n");

	else
		printf("NOTAG %s%s", fpath, tflag == FTW_D ? "/\n" : "\n");

	/* show directory entry instead of its contents */
	if (ftwb->level == 0 && fstool_args->dironly)
		return FTW_STOP;

	/* do not recurse but display directory entries */
	if (tflag == FTW_D && ftwb->level > 0 && !fstool_args->recurse)
		return FTW_SKIP_SUBTREE;

	return FTW_CONTINUE;
}
