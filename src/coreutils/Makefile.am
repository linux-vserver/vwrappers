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

if ENABLE_COREUTILS

INCLUDES = -I$(top_srcdir)/src/wrapper

sbin_PROGRAMS = vdf \
                vdu \
                vuname \
                vusers \
                vwho

WRAPPER_LIBS = $(top_builddir)/src/wrapper/libwrapper.a

vdf_SOURCES = vdf.c
vdf_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vdu_SOURCES = vdu.c
vdu_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vuname_SOURCES = vuname.c
vuname_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vusers_SOURCES = vusers.c
vusers_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vwho_SOURCES = vwho.c
vwho_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

endif
