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

if ENABLE_PROCPS

INCLUDES = -I$(top_srcdir)/src/wrapper

sbin_PROGRAMS = vfuser \
                vkillall \
                vpgrep \
                vpkill \
                vpmap \
                vps \
                vpstree \
                vskill \
                vsnice \
                vstat \
                vtop \
                vuptime \
                vw

WRAPPER_LIBS = $(top_builddir)/src/wrapper/libwrapper.a

vfuser_SOURCES = vfuser.c
vfuser_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vkillall_SOURCES = vkillall.c
vkillall_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vpgrep_SOURCES = vpgrep.c
vpgrep_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vpkill_SOURCES = vpkill.c
vpkill_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vpmap_SOURCES = vpmap.c
vpmap_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vps_SOURCES = vps.c
vps_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vpstree_SOURCES = vpstree.c
vpstree_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vskill_SOURCES = vskill.c
vskill_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vsnice_SOURCES = vsnice.c
vsnice_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vstat_SOURCES = vstat.c
vstat_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vtop_SOURCES = vtop.c
vtop_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vuptime_SOURCES = vuptime.c
vuptime_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vw_SOURCES = vw.c
vw_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

endif
