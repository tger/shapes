# This file is part of Shapes.
#
# Shapes is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# Shapes is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Shapes.  If not, see <http://www.gnu.org/licenses/>.
#
# Copyright 2008 Gustaf Hendeby

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Shapes, 0.4.3, tidefelt@isy.liu.se)
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([source/Shapes_Kernel_decls.h])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_FLEX(2.5.33)
AC_PROG_BISON(2.3)
AC_PROG_COMPARE(6.4.0)
AM_PATH_SAXON(9)
AC_PROG_LIBTOOL

# Set compiler options
AC_DEBUG
AC_PROFILE
AC_OPTIMIZATION

# Checks for libraries.
AM_ICONV()
AH_VERBATIM([ICONV_CAST],
[/* Macro to handle different const convention in iconv calls */
#define ICONV_CAST( ptr ) const_cast< ICONV_CONST char ** > ( ptr )
])
AC_CHECK_LIB([c], [close])
AM_PATH_GSL([1.9],, AC_MSG_ERROR(GSL not found))
AC_CHECK_LIB([m], [cos])
AM_PATH_ZLIB()
AC_CHECK_NANSIGNAL()


# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([inttypes.h libintl.h limits.h malloc.h stddef.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor memchr memset pow rint sqrt strchr strdup strrchr strtol])


htmldir='${pkgdatadir}/doc/html'
AC_SUBST(htmldir)


AC_CONFIG_FILES([Makefile source/Makefile resources/Makefile
                 test/Makefile test/text/Makefile test/graphics/Makefile
                 doc/Makefile doc/tools/Makefile
                 doc/tools/ssi/Makefile doc/tools/postproc/Makefile
                 doc/tools/xhtml-deps/Makefile doc/build/Makefile
                 doc/build/man/Makefile doc/build/html/Makefile doc/build/proj-web/Makefile])
AC_OUTPUT

if test "X$COMPARE" != "Xyes"
then
  echo "\
WARNING:  The program \`compare' was not found, or its version too low.  Without it, errors in the tests are less likely to be detected."
fi

if test "X$HAVE_SAXON" != "X1"
then
  echo "\
WARNING: The program \`Saxon' was not found, without it installed the documentation cannot be rebuilt."
fi
