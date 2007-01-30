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

if ENABLE_UTIL_LINUX

INCLUDES = -I$(top_srcdir)/src/wrapper

sbin_PROGRAMS = vdmesg \
                vipcrm \
                vipcs \
                vmount \
                vrenice \
                vumount

WRAPPER_LIBS = $(top_builddir)/src/wrapper/libwrapper.a

vdmesg_SOURCES = vdmesg.c
vdmesg_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vipcrm_SOURCES = vipcrm.c
vipcrm_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vipcs_SOURCES = vipcs.c
vipcs_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vmount_SOURCES = vmount.c
vmount_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

vrenice_SOURCES = vrenice.c
vrenice_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS) $(WRAPPER_LIBS)

vumount_SOURCES = vumount.c
vumount_LDADD = $(VSERVER_LIBS) $(LUCID_LIBS)

endif
