# RHSrvAny - Turn any Windows program into a Windows service.
# Written by Yuval Kashtan.
# Copyright (C) 2010 Red Hat Inc.
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# Note that this program only makes sense when cross-compiled as a
# Windows program.  Therefore you probably need to use something like
# the Fedora Windows cross-compiler and invoke 'mingw32-configure'
# instead of the normal './configure' command.

AC_INIT([RHSrvAny],[1.0.0])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CPP

dnl If the host is Windows, define _WIN32_WINNT and others.
AC_CANONICAL_HOST
case $host in
    *mingw*|*cygwin*|*win32*)
        CFLAGS="$CFLAGS -D_WIN32_WINNT=0x0500 -DUNICODE -D_UNICODE" ;;
esac

dnl Check for headers which are not present in some versions of MinGW.
AC_CHECK_HEADERS([strsafe.h])

dnl Check for functions which are not present in some versions of MinGW.
AC_CHECK_FUNCS([swprintf_s StringCchPrintf])

dnl Produce output files.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile RHSrvAny/Makefile])
AC_OUTPUT
