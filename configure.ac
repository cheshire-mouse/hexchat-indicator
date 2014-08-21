AC_INIT([xchat-indicator],[0.3.11],[http://launchpad.net/xchat-indicator],[xchat-indicator])

AC_PREREQ([2.60])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([configure.ac])

AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

GLIB_REQUIRED=2.18.0
GTK_REQUIRED=2.14.0
UNITY_REQUIRED=3.4.0

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_C99
AC_PROG_CPP
AM_PROG_AS
AC_PROG_LIBTOOL
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

AC_C_CONST
AC_TYPE_SIZE_T
AC_EXEEXT
AC_HEADER_STDC

AC_CHECK_FUNCS(snprintf vsnprintf)
AC_CHECK_FUNCS(memrchr strtoull atoll)

PKG_PROG_PKG_CONFIG

# Hard dependencies
PKG_CHECK_MODULES([DEPENDENCIES], [
                  gtk+-2.0 >= $GTK_REQUIRED
                  messaging-menu
                  ])
AC_SUBST([DEPENDENCIES_CFLAGS])
AC_SUBST([DEPENDENCIES_LIBS])
# -----------------------------------------------------------
# libunity
# -----------------------------------------------------------
AC_ARG_ENABLE(libunity,
		AS_HELP_STRING([--enable-libunity=@<:@no/yes/auto@:>@],
			[build libunity support]), ,
			enable_libunity=auto)

if test "x$enable_unity" != "xno"; then
   PKG_CHECK_MODULES(UNITY,
   [
     unity >= $UNITY_REQUIRED
   ], have_unity="yes", have_unity="no")

   if test "x$have_unity" = "xyes"; then
      AC_DEFINE(HAVE_UNITY, 1, [Define if you have unity])
   fi
else
   have_unity=no
fi

if test "x$enable_unity" = "xyes" -a "x$have_unity" != "xyes"; then
   AC_MSG_ERROR([Could not find libunity.])
fi

AM_CONDITIONAL(HAVE_UNITY, test "x$have_unity" = "xyes")
AC_SUBST([UNITY_CFLAGS])
AC_SUBST([UNITY_LIBS])

# warning flags
AM_CPPFLAGS="$AM_CPPFLAGS $DISABLE_DEPRECATED"
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS -fgnu89-inline"

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

# various and sundry defines and flags
AC_DEFINE(_GNU_SOURCE, [], [Use GNU source])

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT

echo
echo $PACKAGE $VERSION
echo

echo prefix................. : ${prefix}
echo libunity............... : ${have_unity}
