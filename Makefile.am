## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = yapteaparprfotci

CFLAGS = @CFLAGS@
AM_CPPFLAGS = -I$(top_srcdir)/include

LDADD = @LIBOBJS@

yapteaparprfotci_SOURCES = yapteaparprfotci.c giffunc.c gifread.c clp.c

yapteaparprfotci_DEPENDENCIES = @LIBOBJS@

EXTRA_DIST = README.md NEWS.md 070.GIF 111.GIF 165.GIF 224.GIF 351.GIF \
	include/lcdf/inttypes.h include/lcdf/clp.h include/lcdfgif/gif.h

.PHONY: srclinks
