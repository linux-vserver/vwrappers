## Copyright 2006 Benedikt Böhm <hollow@gentoo.org>
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA

if ENABLE_FSTOOLS

noinst_HEADERS = fstool.h

sbin_PROGRAMS = chxid \
                lsxid \
                setattr \
                showattr

chxid_SOURCES = chxid.c main.c
chxid_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

lsxid_SOURCES = lsxid.c main.c
lsxid_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

setattr_SOURCES = setattr.c main.c
setattr_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

showattr_SOURCES = showattr.c main.c
showattr_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

endif
