#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([tpool], [1.0], [generalpenguin89@gmail.com])
AC_CONFIG_SRCDIR([src/libtpool.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])
LT_INIT([disable-static pic-only])
AC_CONFIG_FILES([
	Makefile
])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC_STDC
AS_CASE([$host],
	[*-*-linux-gnu], [CC="$CC -pthread"],
	[])

# Checks for libraries.
AC_SEARCH_LIBS([pthread_create], [pthread], [], [AC_MSG_ERROR([dnl
"POSIX threads support is required for EXS library"])])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strerror])

my_CFLAGS="-Wall -Wextra"
AC_SUBST([my_CFLAGS])

AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE $VERSION
        =====

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        includedir:             ${includedir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}
])
