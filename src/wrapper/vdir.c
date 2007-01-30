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

#include <stdlib.h>
#include <vserver.h>

#include <lucid/mem.h>
#include <lucid/str.h>

#include "wrapper.h"

char *lookup_vdir(xid_t xid, char *vdir, size_t len)
{
	char *p;
	vx_uname_t uname;

	uname.id = VHIN_CONTEXT;

	if (vx_uname_get(xid, &uname) == -1)
		return NULL;

	else
		p = str_chr(uname.value, ':', str_len(uname.value)) + 1;
	
	if (p) {
		if (str_len(p) < len)
			len = str_len(p);

		mem_cpy(vdir, p, len);
	}

	return p ? vdir : NULL;
}
