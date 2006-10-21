# Copyright 2006 Benedikt Böhm <hollow@gentoo.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

# Generic configuration
AC_PREREQ(2.59)
AC_INIT(vwrappers, 1.0, hollow@gentoo.org)
AC_CONFIG_HEADER([config.h])
AC_GNU_SOURCE

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# Automake
AM_INIT_AUTOMAKE([1.9 gnu dist-bzip2])
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_ARG_ENABLE([coreutils],
              [AC_HELP_STRING([--disable-coreutils],
                              [Disable the coreutil wrappers])],
              [case "${enableval}" in
                yes) enable_coreutils=true;;
                no)  enable_coreutils=false;;
                *)   AC_MSG_ERROR(bad value ${enableval} for --enable-coreutils);;
              esac],
              [enable_coreutils=true])
AM_CONDITIONAL(ENABLE_COREUTILS, test x$enable_coreutils = xtrue)

AC_ARG_ENABLE([fstools],
              [AC_HELP_STRING([--disable-fstools],
                              [Disable the filesystem tools])],
              [case "${enableval}" in
                yes) enable_fstools=true;;
                no)  enable_fstools=false;;
                *)   AC_MSG_ERROR(bad value ${enableval} for --enable-fstools);;
              esac],
              [enable_fstools=true])
AM_CONDITIONAL(ENABLE_FSTOOLS, test x$enable_fstools = xtrue)

AC_ARG_ENABLE([procps],
              [AC_HELP_STRING([--disable-procps],
                              [Disable the procps wrappers])],
              [case "${enableval}" in
                yes) enable_procps=true;;
                no)  enable_procps=false;;
                *)   AC_MSG_ERROR(bad value ${enableval} for --enable-procps);;
              esac],
              [enable_procps=true])
AM_CONDITIONAL(ENABLE_PROCPS, test x$enable_procps = xtrue)

AC_ARG_ENABLE([sysvinit],
              [AC_HELP_STRING([--disable-sysvinit],
                              [Disable the sysvinit wrappers])],
              [case "${enableval}" in
                yes) enable_sysvinit=true;;
                no)  enable_sysvinit=false;;
                *)   AC_MSG_ERROR(bad value ${enableval} for --enable-sysvinit);;
              esac],
              [enable_sysvinit=true])
AM_CONDITIONAL(ENABLE_SYSVINIT, test x$enable_sysvinit = xtrue)

AC_ARG_ENABLE([util-linux],
              [AC_HELP_STRING([--disable-util-linux],
                              [Disable the util-linux wrappers])],
              [case "${enableval}" in
                yes) enable_util_linux=true;;
                no)  enable_util_linux=false;;
                *)   AC_MSG_ERROR(bad value ${enableval} for --enable-util-linux);;
              esac],
              [enable_util_linux=true])
AM_CONDITIONAL(ENABLE_UTIL_LINUX, test x$enable_util_linux = xtrue)


# Checks for libraries.
AC_CHECK_LIB(vserver, vx_get_stat,
	VSERVER_LIBS="-lvserver", AC_MSG_ERROR([libvserver >= 2.0 not found]),)

AC_SUBST(VSERVER_LIBS)

AC_CHECK_LIB(ucid, str_check,
	LUCID_LIBS="-lucid", AC_MSG_ERROR([lucid not found]),)

AC_SUBST(LUCID_LIBS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Compiler settings
CFLAGS="${CFLAGS} -std=gnu99 -pedantic"
CFLAGS="${CFLAGS} -Wall -Wpointer-arith -Wcast-qual -Winline"
CFLAGS="${CFLAGS} -Wredundant-decls -Wcast-align -Wno-unused-parameter"

# Site settings

# Final info page
AC_CONFIG_COMMANDS_PRE([SUMMARY="$PACKAGE_STRING configured successfully:

                       CC: $CC ($($CC --version | head -n1))
                 CPPFLAGS: '$CPPFLAGS'
                   CFLAGS: '$CFLAGS'
                    build: $build
                     host: $host
                   target: $target
                   prefix: $prefix
"])

# Output
AC_CONFIG_FILES([Makefile
                 src/coreutils/Makefile
                 src/fstools/Makefile
                 src/procps/Makefile
                 src/sysvinit/Makefile
                 src/util-linux/Makefile
                 src/wrapper/Makefile
                 src/Makefile])

AC_OUTPUT

AC_MSG_NOTICE([$SUMMARY])
