dnl Process this file with aclocal, autoconf and automake.

AC_INIT(whitenoise, 1.0.2)
AC_CONFIG_HEADERS(config.h)
AC_PREFIX_DEFAULT(/usr/local)

dnl Options

AC_ARG_WITH(arts-lib,  [  --with-arts-lib=DIR      Specify where to find libartsc], LIBARTS_LIBDIR=$withval)
AC_ARG_WITH(arts-inc,  [  --with-arts-inc=DIR      Specify where to find libartsc's header], LIBARTS_INCDIR=$withval)

AC_ARG_WITH(fftw3-lib, [  --with-fftw3-lib=DIR     Specify where to find libfftw3], LIBFFTW_LIBDIR=$withval)
AC_ARG_WITH(fftw3-inc, [  --with-fftw3-inc=DIR     Specify where to find libfftw3's header], LIBFFTW_INCDIR=$withval)

AC_ARG_ENABLE(arts,    [  --disable-arts           Disable support for aRts], enable_arts=$enableval, enable_arts=yes)
AC_ARG_ENABLE(fftw,    [  --disable-fftw           Disable support for FFTW], enable_fftw=$enableval, enable_fftw=yes)

AC_PROG_CC

dnl *****************************
dnl * check for aRts support    *
dnl *****************************
if test "x$enable_arts" = "xyes"; then
   old_cppflags="$CPPFLAGS"
   if ! test -z "$LIBARTS_INCDIR"; then
      CPPFLAGS="$CPPFLAGS -I$LIBARTS_INCDIR"
   fi
   AC_CHECK_HEADER(artsc.h, arts_ok=yes, arts_ok=no)
   CPPFLAGS="$old_cppflags"

   if test "x$arts_ok" = "xyes"; then
      old_libs="$LIBS"
      old_ldflags="$LDFLAGS"
      if ! test -z "$LIBARTS_LIBDIR"; then
         LDFLAGS="$LDFLAGS -L$LIBARTS_LIBDIR"
      fi
      AC_CHECK_LIB(artsc, arts_write, arts_ok=yes, arts_ok=no)
      LIBS="$old_libs"
      LDFLAGS="$old_ldflags"
   fi

   if test "x$arts_ok" = "xyes"; then
      LIBARTS_LIBS="-lartsc"
      if test -n "$LIBARTS_LIBDIR"; then
         LIBARTS_LDFLAGS="-L$LIBARTS_LIBDIR"
      fi
      if test -n "$LIBARTS_INCDIR"; then
         LIBARTS_CPPFLAGS="-I$LIBARTS_INCDIR"
      fi
   else
      AC_MSG_WARN([No libartsc found; disabling aRts audio output support.])
   fi
fi

if test "x$arts_ok" = "xyes"; then
   AC_DEFINE([HAS_ARTS], [], [Enable aRts support])
fi



dnl *****************************
dnl * check for FFTW support    *
dnl *****************************
if test "x$enable_fftw" = "xyes"; then
   old_cppflags="$CPPFLAGS"
   if ! test -z "$LIBFFTW_INCDIR"; then
      CPPFLAGS="$CPPFLAGS -I$LIBFFTW_INCDIR"
   fi
   AC_CHECK_HEADER(fftw3.h, fftw_ok=yes, fftw_ok=no)
   CPPFLAGS="$old_cppflags"

   if test "x$fftw_ok" = "xyes"; then
      old_libs="$LIBS"
      old_ldflags="$LDFLAGS"
      if ! test -z "$LIBFFTW_LIBDIR"; then
         LDFLAGS="$LDFLAGS -L$LIBFFTW_LIBDIR"
      fi
      AC_CHECK_LIB(fftw3, fftw_execute, fftw_ok=yes, fftw_ok=no)
      LIBS="$old_libs"
      LDFLAGS="$old_ldflags"
   fi

   if test "x$fftw_ok" = "xyes"; then
      LIBFFTW_LIBS="-lfftw3"
      if test -n "$LIBFFTW_LIBDIR"; then
         LIBFFTW_LDFLAGS="-L$LIBFFTW_LIBDIR"
      fi
      if test -n "$LIBFFTW_INCDIR"; then
         LIBFFTW_CPPFLAGS="-I$LIBFFTW_INCDIR"
      fi
   else
      AC_MSG_WARN([No libfftw3 found; disabling FFTW-based plotting support.])
   fi
fi

if test "x$fftw_ok" = "xyes"; then
   AC_DEFINE([HAS_FFTW3], [], [Enable FFTW plotting support])
fi

AC_CHECK_LIB(m, sin)
AC_CHECK_LIB(asound, snd_pcm_open)

dnl substitutions
AC_SUBST(LIBARTS_LIBS)
AC_SUBST(LIBARTS_CPPFLAGS)
AC_SUBST(LIBARTS_LDFLAGS)
AC_SUBST(LIBFFTW_LIBS)
AC_SUBST(LIBFFTW_CPPFLAGS)
AC_SUBST(LIBFFTW_LDFLAGS)

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

dnl ----------------------
dnl Show configure summary
dnl ----------------------
echo ""
echo "Finished configure:"
echo "-----------------------------------"
echo "   aRts audio support   : $arts_ok";
echo "   FFTW plotting support: $fftw_ok";
echo ""
echo "Run configure --help for explanation of these options,"
echo "otherwise type 'make' to build whitenoise."

# arch-tag: DO_NOT_CHANGE_edef1604-8eff-4661-8106-bb48f9f793d7 
