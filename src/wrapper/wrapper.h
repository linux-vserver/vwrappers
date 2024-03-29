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

#ifndef _WRAPPER_H
#define _WRAPPER_H

#include <vserver.h>

char *lookup_vdir(xid_t xid, char *vdir, size_t len);

int default_wrapper(int argc, char **argv, char *proc, int needxid);

#define DEFAULT_WRAPPER(PROC, NEEDXID) \
int main(int argc, char **argv) { \
	return default_wrapper(argc, argv, PROC, NEEDXID); \
}

#endif
