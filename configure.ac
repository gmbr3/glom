## Copyright (c) 2009, 2010  Openismus GmbH  <http://www.openismus.com/>
##
## This file is part of Glom.
##
## Glom is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published
## by the Free Software Foundation, either version 2 of the License,
## or (at your option) any later version.
##
## Glom is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT([Glom],[1.33.1],[http://bugzilla.gnome.org/enter_bug.cgi?product=Glom],[glom])
AC_PREREQ(2.63)

AC_CONFIG_SRCDIR([glom/main.cc])
AC_CONFIG_MACRO_DIR([macros])
AC_CONFIG_HEADERS([config.h glom/libglom/libglom_config.h])

AM_INIT_AUTOMAKE([1.10 -Wno-portability no-define nostdinc tar-ustar])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE
AC_ARG_VAR([ACLOCAL_FLAGS], [aclocal flags, e.g. -I <macro dir>])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX

AX_CXX_COMPILE_STDCXX([14], [noext], [mandatory])

LT_PREREQ([2.2.0])
LT_INIT([disable-static win32-dll])

# Used for the install directories for headers.  The same number is used in
# the library name, which must be kept in sync, but variables can't be used
# with that.
AC_SUBST([GLOM_ABI_VERSION], [1.34])
AC_DEFINE_UNQUOTED([GLOM_ABI_VERSION], ["$GLOM_ABI_VERSION"],
		   [Define to the Glom ABI version string.])

# Python modules can't be called glom-1.x, so we use underlines.
AC_SUBST([GLOM_ABI_VERSION_UNDERLINED], ["AS_TR_SH([$GLOM_ABI_VERSION])"])
AC_DEFINE_UNQUOTED([GLOM_ABI_VERSION_UNDERLINED], ["$GLOM_ABI_VERSION_UNDERLINED"],
                   [Define to the Glom ABI version with '.' replaced by '_'.])


# libgettext-po changed its API, changing the error handler struct
# from po_error_handler to po_xerror_handler:
AC_CACHE_CHECK([for po_xerror_handler], [glom_cv_has_po_xerror_handler],
       [AC_CHECK_MEMBER([struct po_xerror_handler.xerror],
                        [glom_cv_has_po_xerror_handler=yes],
                        [glom_cv_has_po_xerror_handler=no],
                        [#include <gettext-po.h>])])

AS_IF([test "x$glom_cv_has_po_xerror_handler" = xyes],
  [AC_DEFINE([HAVE_GETTEXTPO_XERROR], [1],
  [Define if libgettextpo provides the new po_xerror_handler struct.])])


# i18n
IT_PROG_INTLTOOL([0.35])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

AC_SUBST([GETTEXT_PACKAGE], [glom])
AC_DEFINE([GETTEXT_PACKAGE], [PACKAGE_TARNAME], [Define to the gettext package name.])

# TODO: Make this work, so we have GLOM_LOCALEDIR in config.h,
# instead of defining it via -D in the Makefile.am:
# AC_DEFINE([GLOM_LOCALEDIR], [$localedir], [The location of the gettext catalogs as defined by intltool.])

# Linking with libdl isn't needed on various non-Linux platforms,
# eventhough they do provide dlopen(3).
AC_CHECK_LIB([dl], [dlopen], AC_SUBST([DL_LIB], ["-ldl"]))

AC_MSG_CHECKING([whether to enable Windows specific options])
AS_CASE([$host_os], [mingw*], [glom_host_win32=yes], [glom_host_win32=no])
AC_MSG_RESULT([$glom_host_win32])

AS_IF([test "x$glom_host_win32" = xyes],
      [AC_CHECK_TOOL([WINDRES], [windres])])

AM_CONDITIONAL([HOST_WIN32], [test "x$glom_host_win32" = xyes])

AC_ARG_ENABLE([glom-ui],
              [AS_HELP_STRING([--enable-glom-ui],
                              [build the glom UI application instead of just libglom. The command-line utilities are always built. [default=yes]])],
              [glom_enable_ui=$enableval],
              [glom_enable_ui=yes])
AM_CONDITIONAL([GLOM_ENABLE_UI], [test "x$glom_enable_ui" = xyes])


AC_ARG_ENABLE([client-only],
              [AS_HELP_STRING([--enable-client-only],
                              [build without developer mode and self-hosting])],
              [glom_enable_client_only=$enableval],
              [glom_enable_client_only=no])


AM_CONDITIONAL([GLOM_ENABLE_CLIENT_ONLY], [test "x$glom_enable_client_only" = xyes])

AS_IF([test "x$glom_enable_client_only" = xyes],
         AC_DEFINE([GLOM_ENABLE_CLIENT_ONLY], [1],
                 [Define to disable support for self-hosting and developer mode.])])

AC_ARG_ENABLE([ui-tests],
              [AS_HELP_STRING([--enable-ui-tests],
                              [Run UI tests during make check. [default=yes]])],
              [glom_enable_ui_tests=$enableval],
              [glom_enable_ui_tests=yes])
AM_CONDITIONAL([GLOM_ENABLE_UI_TESTS], [test "x$glom_enable_ui_tests" = xyes])


# SQLite support:
AC_ARG_ENABLE([sqlite],
              [AS_HELP_STRING([--enable-sqlite],
                              [Allow creation of SQLite databases and opening
                               of documents using SQLite databases. This
                               disables some functionality and is intended
                               only for embedded use.])],
              [glom_enable_sqlite=$enableval],
              [glom_enable_sqlite=no])

AM_CONDITIONAL([GLOM_ENABLE_SQLITE], [test "x$glom_enable_sqlite" = xyes])

AS_IF([test "x$glom_enable_sqlite" = xyes],
      [AC_DEFINE([GLOM_ENABLE_SQLITE], [1],
                 [Whether to enable support for SQLite databases.])])


# MySQL support:
AC_ARG_ENABLE([mysql],
              [AS_HELP_STRING([--disable-mysql],
                              [do not build with support for MySQL databases])],
              [glom_enable_mysql=$enableval],
              [glom_enable_mysql=no])

AM_CONDITIONAL([GLOM_ENABLE_MYSQL], [test "x$glom_enable_mysql" = xyes])

AS_IF([test "x$glom_enable_mysql" = xyes],
      [AC_DEFINE([GLOM_ENABLE_MYSQL], [1],
                 [Whether to enable support for MySQL databases.])])


# PostgreSQL support:
AC_ARG_ENABLE([postgresql],
              [AS_HELP_STRING([--disable-postgresql],
                              [do not build with support for PostgreSQL databases])],
              [glom_enable_postgresql=$enableval],
              [glom_enable_postgresql=yes])

AM_CONDITIONAL([GLOM_ENABLE_POSTGRESQL], [test "x$glom_enable_postgresql" = xyes])

AS_IF([test "x$glom_enable_postgresql" = xyes],
      [AC_DEFINE([GLOM_ENABLE_POSTGRESQL], [1],
                 [Whether to enable support for PostgreSQL databases.])])


# Libraries used by libglom:
REQUIRED_LIBGLOM_LIBS='giomm-2.4 >= 2.47.4 libxml++-3.0 >= 3.0.0 libxslt >= 1.1.10 pygobject-3.0 >= 2.29.0 libgdamm-5.0 >= 5.99.1 libgda-5.0 >= 6.0.0 libgda-postgres-5.0 libgda-postgres-5.0 libgda-mysql-5.0 libarchive >= 3.0'

AS_IF([test "x$glom_host_win32" != xyes],
      [REQUIRED_LIBGLOM_LIBS="$REQUIRED_LIBGLOM_LIBS libepc-1.0 >= 0.4.0"])

# Libraries used by Glom:
REQUIRED_GLOM_LIBS="$REQUIRED_LIBGLOM_LIBS gtkmm-3.0 >= 3.22.0 sigc++-2.0 >= 2.9.2 goocanvasmm-2.0 >= 1.90.11 goocanvas-2.0 >= 2.0.1 evince-view-3.0"

# Do not require iso-codes in client-only mode, or on Windows:
# TODO: Package iso-codes for Windows?
AS_IF([test "x$glom_enable_client_only" != xyes && test "x$glom_host_win32" != xyes],
      [REQUIRED_GLOM_LIBS="$REQUIRED_GLOM_LIBS iso-codes"])

# Do not require gtksourceviewmm in client only mode
AS_IF([test "x$glom_enable_client_only" != xyes],
      [REQUIRED_GLOM_LIBS="$REQUIRED_GLOM_LIBS gtksourceviewmm-4.0 >= 3.18.0"])

AS_IF([test "x$glom_enable_sqlite" = xyes],
      [REQUIRED_GLOM_LIBS="$REQUIRED_GLOM_LIBS libgda-sqlite-6.0"])

#TODO: Remove this check, because we checked again later anyway,
#because we add REQUIRED_LIBGLOM_LIBS to REQUIRED_GLOM_LIBS?
PKG_CHECK_MODULES([LIBGLOM], [$REQUIRED_LIBGLOM_LIBS])

#Disable the checks for UI dependencies if not building the UI:
AS_IF([test "x$glom_enable_ui" = xno],
      [REQUIRED_GLOM_LIBS="$REQUIRED_LIBGLOM_LIBS"])

PKG_CHECK_MODULES([GLOM], [$REQUIRED_GLOM_LIBS])

MM_PKG_CONFIG_SUBST([GLIB_GENMARSHAL], [--variable=glib_genmarshal glib-2.0])

# Get the location of the ISO-Codes (currencies, languages) files:
MM_PKG_CONFIG_SUBST([ISO_CODES_PREFIX], [--variable=prefix iso-codes])
AC_DEFINE_UNQUOTED([ISO_CODES_PREFIX], ["$ISO_CODES_PREFIX"],
                   [Define to the installation prefix of the iso-codes module.])

# This is used in one test:
PKG_CHECK_MODULES([GDKMM], [gdkmm-3.0 >= 3.4.0])

# Allow use of mm-common macros for the warnings option and for installing developer documentation.
MM_PREREQ([0.9.5])
MM_INIT_MODULE([libglom-1.32])

# Copy the mm-common .pl scripts into docs/,
# and use them from there,
# so we can dist them to avoid a tarball-build dependency.
MM_CONFIG_DOCTOOL_DIR([docs/libglom_reference])


# Evaluate the --enable-warnings=level option.

# We don't use -pedantic with the C sources, to avoid these warnings in eggmarshalers.c:
# "ISO C forbids conversion of object pointer to function pointer type"
AC_LANG([C])
MM_ARG_ENABLE_WARNINGS([GLOM_WFLAGS],
                       [-Wall],
                       [-Wall -Wextra -Wformat-security -Wno-missing-field-initializers -DGSEAL_ENABLE],
                       [G GDK GDK_PIXBUF CAIRO PANGO GTK GDA])

# We use  -Wno-long-long because the Python headers use long long (at least in Python 2.7).
AC_LANG([C++])
MM_ARG_ENABLE_WARNINGS([GLOM_WXXFLAGS],
                       [-Wall],
                       [-pedantic -Wall -Wextra -Wsuggest-override -Wshadow -Wformat-security -Wformat=2 -Wno-long-long -Wno-missing-field-initializers -DGSEAL_ENABLE],
                       [G GDK GDK_PIXBUF GTK GDA GLIBMM GDKMM CAIROMM PANGOMM GTKMM SIGCXX LIBGDAMM])

AC_CHECK_FUNCS([strptime])

# Get the path to python, to define pyexecdir,
# so we can use pyexec_LTLIBRARIES in the .am files:
# (This sets PYTHON_VERSION, used by AX_PYTHON_DEVEL later.)
#
# To specify a particular python version you must pass PYTHON.
# For instance, ./configure PYTHON=python3.4 --prefix=/opt/gnome
AM_PATH_PYTHON([3.0])

# Get the compiler and linker flags for embedding Python.
# This uses the PYTHON_VERSION set by AM_PATH_PYTHON above.
#
# See http://www.gnu.org/software/autoconf-archive/ax_python_devel.html
AX_PYTHON_DEVEL([>= '3.0.0'])

# Get the CFLAGS and LIBS for boost::python.
# To use a non-default Boost::Python library, for instance to use
# the version for Python3, if you already have set, for instance, PYTHON=python3.3,
# you'll need to pass something like this to configure to make it
# link to the correct library (Yes, this is tedious).
#   --with-boost-python=boost_python-mt-py33
#
# See http://www.gnu.org/software/autoconf-archive/ax_boost_base.html
AX_BOOST_BASE
# See http://www.gnu.org/software/autoconf-archive/ax_boost_python.html
AX_BOOST_PYTHON

# Check that ax_boost_python.m4 found something.
# It also defines HAVE_BOOST_PYTHON, but only as a C define for
AS_IF([test -z "$BOOST_PYTHON_LIB"],
  [AC_MSG_ERROR([Cannot find the Boost::Python library.])])

AC_ARG_ENABLE([update-mime-database],
              [AS_HELP_STRING([--disable-update-mime-database],
                              [do not run the update-mime-database utility
                               (mainly useful to package maintainers)])],
              [glom_update_mime_database=$enableval],
              [glom_update_mime_database=yes])

AM_CONDITIONAL([UPDATE_MIME_DATABASE], [test "x$glom_update_mime_database" != xno])


# Locate the directory containing the PostgreSQL utilities, such as the
# postmaster executable, so we can self-host PostgreSQL databases.
AC_ARG_WITH([postgres-utils],
            [AS_HELP_STRING([--with-postgres-utils=DIR],
                            [path to PostgreSQL utilities (overriding pg_config)])],
            [POSTGRES_UTILS_PATH=$withval])

# Path not needed on Windows
AS_IF([test "x$glom_host_win32" != xyes && test "x$glom_enable_client_only" != xyes],
      [AS_CASE([$POSTGRES_UTILS_PATH], [""|no|yes],
[
  # For instance, this is in libpq-dev in Ubuntu:
  AC_CHECK_PROG([PG_CONFIG], [pg_config], [yes], [no])
  AS_IF([test "$PG_CONFIG" = no],
    [AC_MSG_ERROR([The PostgreSQL pg_config utility was not found. Glom needs this to find the other PostgreSQL utilities.])])

  POSTGRES_UTILS_PATH=`pg_config --bindir 2>&AS_MESSAGE_LOG_FD`
  AS_IF(["$POSTGRES_UTILS_PATH/pg_ctl" --version >/dev/null 2>&AS_MESSAGE_LOG_FD],,
        [AC_MSG_ERROR([[
The PostgreSQL utilities could not be found. They are needed for
self-hosting of Glom databases. Please make sure that PostgreSQL
is installed, and if necessary specify the correct directory
explicitly with the --with-postgres-utils option.
]])])])])
AC_DEFINE_UNQUOTED([POSTGRES_UTILS_PATH], ["$POSTGRES_UTILS_PATH"],
                   [Define to the location of the PostgreSQL utilities.])
AC_DEFINE_UNQUOTED([EXEEXT], ["$EXEEXT"],
                   [Define to the file extension of executables on the target.])


# Locate the directory containing the MySQL utilities, such as the
# postmaster executable, so we can self-host MySQL databases.
AC_ARG_WITH([mysql-utils],
            [AS_HELP_STRING([--with-mysql-utils=DIR],
                            [path to MySQL utilities.])],
            [MYSQL_UTILS_PATH=$withval])
# Path not needed on Windows
AS_IF([test "x$glom_host_win32" != xyes && test "x$glom_enable_client_only" != xyes],
      [AS_CASE([$MYSQL_UTILS_PATH], [""|no|yes],
[
  # TODO: Check properly instead of hard-coding /usr/bin
  MYSQL_UTILS_PATH="/usr/bin"
])])

AC_DEFINE_UNQUOTED([MYSQL_UTILS_PATH], ["$MYSQL_UTILS_PATH"],
                   [Define to the location of the MySQL utilities.])

AS_IF([test "x$ENABLE_DOCUMENTATION" != xno],
  [YELP_HELP_INIT])

MM_ARG_ENABLE_DOCUMENTATION
MM_ARG_WITH_TAGFILE_DOC([libstdc++.tag], [mm-common-libstdc++])
MM_ARG_WITH_TAGFILE_DOC([libsigc++-2.0.tag], [sigc++-2.0])
MM_ARG_WITH_TAGFILE_DOC([glibmm-2.4.tag], [glibmm-2.4])
MM_ARG_WITH_TAGFILE_DOC([cairomm-1.0.tag], [cairomm-1.0])
MM_ARG_WITH_TAGFILE_DOC([pangomm-1.4.tag], [pangomm-1.4])

AC_ARG_VAR([SPHINX_BUILD], [path to sphinx-build utility])
AS_IF([test "x$ENABLE_DOCUMENTATION" != xno],
  [AC_PATH_PROG([SPHINX_BUILD], [sphinx-build], [sphinx-build])
   AS_IF([test "x$SPHINX_BUILD" != xsphinx-build],
     [],
     [AC_MSG_ERROR([The documentation build is enabled, but the sphinx-build tool could not be found.])])])

# Check for tar and gzip (used by tar via -z) because we use these when
# creating backups.
# TODO: This lets us provide a path via a configure option, but we just use
# Glib::find_program_in_path(), ignoring that.
# TODO: This should check for GNU tar, as GNU tar features are used during the
# backup process.
AC_CHECK_PROG([GLOM_TAR], [tar], [yes], [no])
AS_IF([test "$GLOM_TAR" = no],
  [AC_MSG_ERROR([tar not found. Glom needs this to create backup files.])])

AC_CHECK_PROG([GLOM_GZIP], [gzip], [yes], [no])
AS_IF([test "$GLOM_GZIP" = no],
  [AC_MSG_ERROR([gzip not found. Glom needs this to create backup files.])])

AC_PATH_PROG([GLOM_MSGFMT], [msgfmt], [no])
AS_IF([test "$GLOM_MSGFMT" = no],
  [AC_MSG_ERROR([msgfmt not found. Glom needs this to test exported .po (gettext) files.])])
AC_DEFINE_UNQUOTED([GLOM_MSGFMT], ["$GLOM_MSGFMT"],
                   [Define to the location of the msgfmt gettext utility.])

# Code testing coverage with gcov
AC_MSG_CHECKING(whether to build with gcov testing)
AC_ARG_ENABLE(gcov, AS_HELP_STRING([--enable-gcov], [Whether to enable gcov testing]),, enable_gcov=no)

if test "x$enable_gcov" = "xyes"; then
  if test "$GCC" = "no"; then
    AC_MSG_ERROR(not compiling with gcc, which is required for gcov testing)
  fi

  AC_PATH_PROG(GCOV, [gcov], [no])
  if test "x$GCOV" = "xno"; then
    AC_MSG_ERROR([gcov was enabled but gcov was not found.])
  fi
  AC_SUBST(GCOV)

  AC_PATH_PROG(LCOV, [lcov], [no])
  if test "x$LCOV" = "xno"; then
    AC_MSG_ERROR([gcov was enabled but lcov was not found.])
  fi
  AC_SUBST(LCOV)

  AC_PATH_PROG(GENHTML, [genhtml], [no])
  if test "x$GENHTML" = "xno"; then
    AC_MSG_ERROR([gcov was enabled but lcov's genhtml was not found.])
  fi
  AC_SUBST(GENHTML)

  GCOV_CFLAGS="-O0 -g -fprofile-arcs -ftest-coverage"
  GCOV_LIBS="-lgcov"

  AC_SUBST(GCOV_CFLAGS)
  AC_SUBST(GCOV_LIBS)
fi

AM_CONDITIONAL(GCOV_ENABLED, test x$enable_gcov = xyes)
AC_MSG_RESULT($enable_gcov)

# GResources
GLIB_COMPILE_RESOURCES=`$PKG_CONFIG --variable glib_compile_resources gio-2.0`
AC_SUBST(GLIB_COMPILE_RESOURCES)

AC_CHECK_PROGS([CLANG_FORMAT], [clang-format clang-format-3.9 clang-format-3.8 clang-format-3.7], [])
AM_CONDITIONAL([HAVE_CLANG_FORMAT], test -n "$CLANG_FORMAT")

AC_CONFIG_FILES([Makefile
                 po/Makefile.in
                 glom.desktop.in
                 glom/libglom/glom-${GLOM_ABI_VERSION}.pc:glom/libglom/glom.pc.in
                 docs/user-guide/Makefile
                 docs/libglom_reference/Makefile
                 docs/libglom_reference/Doxyfile
                 docs/pyglom_reference/Makefile
                 docs/pyglom_reference/conf.py
                 docs/pyglom_reference/index.rst
                 win32/glom.iss])

AC_CONFIG_FILES([tests/test_glom_date_in_locales.sh], [chmod u+x tests/test_glom_date_in_locales.sh])
AC_CONFIG_FILES([tests/test_glom_date_in_locales_lc_time.sh], [chmod u+x tests/test_glom_date_in_locales_lc_time.sh])

AC_OUTPUT
