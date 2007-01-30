// Copyright 2006 Benedikt Böhm <hollow@gentoo.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA

#ifndef _FSTOOL_H
#define _FSTOOL_H

#include <stdint.h>
#include <ftw.h>
#include <vserver.h>
#include <lucid/flist.h>

typedef struct {
	int recurse;
	int dironly;
	xid_t xid;
	uint32_t flags;
	uint32_t mask;
	int errcnt;
} fstool_args_t;

extern const flist32_t iattr_list[];

extern const fstool_args_t *fstool_args;
extern int   errcnt;

void usage(int rc);

int handle_file(const char *fpath, const struct stat *sb,
                int typeflag, struct FTW *ftwbuf);

#endif
