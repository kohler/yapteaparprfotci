dnl Process this file with autoconf to produce a configure script.
AC_INIT([yapteaparprfotci], [0.319198726523948])
AC_CONFIG_SRCDIR([yapteaparprfotci.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_C_CONST
AC_C_INLINE

AC_ARG_ENABLE([warnings],
    [AS_HELP_STRING([--enable-warnings], [compile with -W -Wall])],
    [if test "x$enableval" = xyes; then CC="$CC -W -Wall"; fi])


dnl
dnl directory headers, strerror()
dnl

AC_HEADER_DIRENT
AC_CHECK_FUNCS([strtoul srandomdev])
AC_SEARCH_LIBS(floor, m)


dnl
dnl integer types
dnl

AC_CHECK_HEADERS(inttypes.h, have_inttypes_h=yes, have_inttypes_h=no)
AC_CHECK_HEADERS(sys/types.h, have_sys_types_h=yes, have_sys_types_h=no)

if test $have_inttypes_h = no -a $have_sys_types_h = yes; then
    AC_CACHE_CHECK([for uintXX_t typedefs], ac_cv_uint_t,
    [AC_EGREP_HEADER(dnl
changequote(<<,>>)<<(^|[^a-zA-Z_0-9])uint32_t[^a-zA-Z_0-9]>>changequote([,]),
    sys/types.h, ac_cv_uint_t=yes, ac_cv_uint_t=no)])
fi
if test $have_inttypes_h = no -a $have_sys_types_h = yes -a "$ac_cv_uint_t" = no; then
    AC_CACHE_CHECK([for u_intXX_t typedefs], ac_cv_u_int_t,
    [AC_EGREP_HEADER(dnl
changequote(<<,>>)<<(^|[^a-zA-Z_0-9])u_int32_t[^a-zA-Z_0-9]>>changequote([,]),
    sys/types.h, ac_cv_u_int_t=yes, ac_cv_u_int_t=no)])
fi
if test $have_inttypes_h = yes -o "$ac_cv_uint_t" = yes; then
    :
elif test "$ac_cv_u_int_t" = yes; then
    AC_DEFINE(HAVE_U_INT_TYPES, 1, [Define if you have u_intXX_t types but not uintXX_t types.])
else
    AC_MSG_WARN([
=========================================

Neither uint32_t nor u_int32_t defined by <inttypes.h> or <sys/types.h>!
Assuming "short" has 16 bits and "int" has 32 bits.

=========================================])
    AC_DEFINE(HAVE_FAKE_INT_TYPES, 1, [Define if intXX_t types are not available.])
fi

AC_CHECK_TYPES(uintptr_t, [], [],
[#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
])

AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF([unsigned long])
AC_CHECK_SIZEOF([unsigned int])


dnl
dnl verbatim portions of the header
dnl

AH_TOP([#ifndef YAPTEAPARPRFOTCI_CONFIG_H
#define YAPTEAPARPRFOTCI_CONFIG_H])

AH_BOTTOM([#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prototype strerror() if we don't have it. */
#if !HAVE_STRERROR
char *strerror(int errno);
#endif

#ifdef __cplusplus
}
/* Get rid of a possible inline macro under C++. */
# define inline inline
#endif

#endif /* YAPTEAPARPRFOTCI_CONFIG_H */])


dnl
dnl Output
dnl

AC_OUTPUT(Makefile)
