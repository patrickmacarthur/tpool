EXTRA_DIST =
CLEANFILES =
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CPPFLAGS = \
	-I${top_srcdir}/src/tpool \
	-I${top_srcdir}/src

AM_CFLAGS = \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections

AM_LDFLAGS = \
	-Wl,--gc-sections \
	-Wl,--as-needed

LIBTPOOL_CURRENT=1
LIBTPOOL_REVISION=0
LIBTPOOL_AGE=0

pkginclude_HEADERS = src/tpool/tpool.h
lib_LTLIBRARIES = src/libtpool.la

src_libtpool_la_SOURCES =\
	src/future.c \
	src/libtpool.c \
	src/queue.c \
	src/tpool-private.h

EXTRA_DIST += src/libtpool.sym

src_libtpool_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info $(LIBTPOOL_CURRENT):$(LIBTPOOL_REVISION):$(LIBTPOOL_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/libtpool.sym
src_libtpool_la_DEPENDENCIES = $(top_srcdir)/src/libtpool.sym

TESTS = src/test-libtpool

check_PROGRAMS = src/test-libtpool
src_test_libtpool_SOURCES = src/test-libtpool.c
src_test_libtpool_LDADD = src/libtpool.la
