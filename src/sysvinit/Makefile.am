## Copyright 2006 Benedikt Böhm <hollow@gentoo.org>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the
## Free Software Foundation, Inc.,
## 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

if ENABLE_SYSVINIT

sbin_PROGRAMS = vhalt \
                vkillall5 \
                vlastb \
                vlast \
                vpidof \
                vpoweroff \
                vreboot \
                vrunlevel \
                vshutdown \
                vtelinit

vhalt_SOURCES = vhalt.c
vhalt_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vkillall5_SOURCES = vkillall5.c
vkillall5_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vlastb_SOURCES = vlastb.c
vlastb_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vlast_SOURCES = vlast.c
vlast_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vpidof_SOURCES = vpidof.c
vpidof_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vpoweroff_SOURCES = vpoweroff.c
vpoweroff_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vreboot_SOURCES = vreboot.c
vreboot_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vrunlevel_SOURCES = vrunlevel.c
vrunlevel_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vshutdown_SOURCES = vshutdown.c
vshutdown_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vtelinit_SOURCES = vtelinit.c
vtelinit_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

endif