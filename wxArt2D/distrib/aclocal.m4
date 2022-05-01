dnl ---------------------------------------------------------------------------
dnl Macros for wxWindows detection. Typically used in configure.in as:
dnl
dnl 	AC_ARG_ENABLE(...)
dnl 	AC_ARG_WITH(...)
dnl	...
dnl	AM_OPTIONS_WXCONFIG
dnl	...
dnl	...
dnl	AM_PATH_WXCONFIG(2.3.4, wxWin=1)
dnl     if test "$wxWin" != 1; then
dnl        AC_MSG_ERROR([
dnl     	   wxWindows must be installed on your system
dnl     	   but wx-config script couldn't be found.
dnl     
dnl     	   Please check that wx-config is in path, the directory
dnl     	   where wxWindows libraries are installed (returned by
dnl     	   'wx-config --libs' command) is in LD_LIBRARY_PATH or
dnl     	   equivalent variable and wxWindows version is 2.3.4 or above.
dnl        ])
dnl     fi
dnl     CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
dnl     CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
dnl     CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
dnl     
dnl     LDFLAGS="$LDFLAGS $WX_LIBS"
dnl ---------------------------------------------------------------------------

dnl ---------------------------------------------------------------------------
dnl AM_OPTIONS_WXCONFIG
dnl
dnl adds support for --wx-prefix, --wx-exec-prefix and --wx-config 
dnl command line options
dnl ---------------------------------------------------------------------------

AC_DEFUN(AM_OPTIONS_WXCONFIG,
[
   AC_ARG_WITH(wx-prefix, AC_HELP_STRING([--with-wx-prefix=PREFIX],[Prefix where wxWindows is installed (optional)]),
               wx_config_prefix="$withval", wx_config_prefix="")
   AC_ARG_WITH(wx-exec-prefix,AC_HELP_STRING([--with-wx-exec-prefix=PREFIX],[Exec prefix where wxWindows is installed (optional)]),
               wx_config_exec_prefix="$withval", wx_config_exec_prefix="")
   AC_ARG_WITH(wx-config,AC_HELP_STRING([--with-wx-config=CONFIG],[wx-config script to use (optional)]),
               wx_config_name="$withval", wx_config_name="")
])

dnl ---------------------------------------------------------------------------
dnl AM_PATH_WXCONFIG(VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Test for wxWindows, and define WX_C*FLAGS, WX_LIBS and WX_LIBS_STATIC
dnl (the latter is for static linking against wxWindows). Set WX_CONFIG_NAME
dnl environment variable to override the default name of the wx-config script
dnl to use. Set WX_CONFIG_PATH to specify the full path to wx-config - in this
dnl case the macro won't even waste time on tests for its existence.
dnl ---------------------------------------------------------------------------

dnl
dnl Get the cflags and libraries from the wx-config script
dnl
AC_DEFUN(AM_PATH_WXCONFIG,
[
  dnl do we have wx-config name: it can be wx-config or wxd-config or ...
  if test x${WX_CONFIG_NAME+set} != xset ; then
     WX_CONFIG_NAME=wx-config
  fi
  if test "x$wx_config_name" != x ; then
     WX_CONFIG_NAME="$wx_config_name"
  fi

  dnl deal with optional prefixes
  if test x$wx_config_exec_prefix != x ; then
     wx_config_args="$wx_config_args --exec-prefix=$wx_config_exec_prefix"
     WX_LOOKUP_PATH="$wx_config_exec_prefix/bin"
  fi
  if test x$wx_config_prefix != x ; then
     wx_config_args="$wx_config_args --prefix=$wx_config_prefix"
     WX_LOOKUP_PATH="$WX_LOOKUP_PATH:$wx_config_prefix/bin"
  fi

  dnl don't search the PATH if WX_CONFIG_NAME is absolute filename
  if test -x "$WX_CONFIG_NAME" ; then
     AC_MSG_CHECKING(for wx-config)
     WX_CONFIG_PATH="$WX_CONFIG_NAME"
     AC_MSG_RESULT($WX_CONFIG_PATH)
  else
     AC_PATH_PROG(WX_CONFIG_PATH, $WX_CONFIG_NAME, no, "$WX_LOOKUP_PATH:$PATH")
  fi

  if test "$WX_CONFIG_PATH" != "no" ; then
    WX_VERSION=""
    no_wx=""

    min_wx_version=ifelse([$1], ,2.2.1,$1)
    AC_MSG_CHECKING(for wxWindows version >= $min_wx_version)

    WX_CONFIG_WITH_ARGS="$WX_CONFIG_PATH $wx_config_args"

    WX_VERSION=`$WX_CONFIG_WITH_ARGS --version`
    wx_config_major_version=`echo $WX_VERSION | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    wx_config_minor_version=`echo $WX_VERSION | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    wx_config_micro_version=`echo $WX_VERSION | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    wx_requested_major_version=`echo $min_wx_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    wx_requested_minor_version=`echo $min_wx_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    wx_requested_micro_version=`echo $min_wx_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    wx_ver_ok=""
    if test $wx_config_major_version -gt $wx_requested_major_version; then
      wx_ver_ok=yes
    else
      if test $wx_config_major_version -eq $wx_requested_major_version; then
         if test $wx_config_minor_version -gt $wx_requested_minor_version; then
            wx_ver_ok=yes
         else
            if test $wx_config_minor_version -eq $wx_requested_minor_version; then
               if test $wx_config_micro_version -ge $wx_requested_micro_version; then
                  wx_ver_ok=yes
               fi
            fi
         fi
      fi
    fi

    if test "x$wx_ver_ok" = x ; then
      no_wx=yes
    else
      WX_LIBS=`$WX_CONFIG_WITH_ARGS --libs`
      WX_LIBS_STATIC=`$WX_CONFIG_WITH_ARGS --static --libs`

      dnl starting with version 2.2.6 wx-config has --cppflags argument
      wx_has_cppflags=""
      if test $wx_config_major_version -gt 2; then
        wx_has_cppflags=yes
      else
        if test $wx_config_major_version -eq 2; then
           if test $wx_config_minor_version -gt 2; then
              wx_has_cppflags=yes
           else
              if test $wx_config_minor_version -eq 2; then
                 if test $wx_config_micro_version -ge 6; then
                    wx_has_cppflags=yes
                 fi
              fi
           fi
        fi
      fi

      if test "x$wx_has_cppflags" = x ; then
         dnl no choice but to define all flags like CFLAGS
         WX_CFLAGS=`$WX_CONFIG_WITH_ARGS --cflags`
         WX_CPPFLAGS=$WX_CFLAGS
         WX_CXXFLAGS=$WX_CFLAGS

         WX_CFLAGS_ONLY=$WX_CFLAGS
         WX_CXXFLAGS_ONLY=$WX_CFLAGS
      else
         dnl we have CPPFLAGS included in CFLAGS included in CXXFLAGS
         WX_CPPFLAGS=`$WX_CONFIG_WITH_ARGS --cppflags`
         WX_CXXFLAGS=`$WX_CONFIG_WITH_ARGS --cxxflags`
         WX_CFLAGS=`$WX_CONFIG_WITH_ARGS --cflags`

         WX_CFLAGS_ONLY=`echo $WX_CFLAGS | sed "s@^$WX_CPPFLAGS *@@"`
         WX_CXXFLAGS_ONLY=`echo $WX_CXXFLAGS | sed "s@^$WX_CFLAGS *@@"`
      fi
    fi

    if test "x$no_wx" = x ; then
       AC_MSG_RESULT(yes (version $WX_VERSION))
       ifelse([$2], , :, [$2])
    else
       if test "x$WX_VERSION" = x; then
	  dnl no wx-config at all
	  AC_MSG_RESULT(no)
       else
	  AC_MSG_RESULT(no (version $WX_VERSION is not new enough))
       fi

       WX_CFLAGS=""
       WX_CPPFLAGS=""
       WX_CXXFLAGS=""
       WX_LIBS=""
       WX_LIBS_STATIC=""
       ifelse([$3], , :, [$3])
    fi
  fi

  AC_SUBST(WX_CPPFLAGS)
  AC_SUBST(WX_CFLAGS)
  AC_SUBST(WX_CXXFLAGS)
  AC_SUBST(WX_CFLAGS_ONLY)
  AC_SUBST(WX_CXXFLAGS_ONLY)
  AC_SUBST(WX_LIBS)
  AC_SUBST(WX_LIBS_STATIC)
  AC_SUBST(WX_VERSION)
])






# Configure paths for FreeType2
# Marcelo Magallon 2001-10-26, based on gtk.m4 by Owen Taylor
#
# serial 2

# AC_CHECK_FT2([MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
# Test for FreeType 2, and define FT2_CFLAGS and FT2_LIBS.
# MINIMUM-VERSION is what libtool reports; the default is `7.0.1' (this is
# FreeType 2.0.4).
#
AC_DEFUN([AC_CHECK_FT2],
  [# Get the cflags and libraries from the freetype-config script
   #
   AC_ARG_WITH(ft-prefix,
     AC_HELP_STRING([--with-ft-prefix=PREFIX],
                    [Prefix where FreeType is installed (optional)]),
     [ft_config_prefix="$withval"],
     [ft_config_prefix=""])

   AC_ARG_WITH(ft-exec-prefix,
     AC_HELP_STRING([--with-ft-exec-prefix=PREFIX],
                    [Exec prefix where FreeType is installed (optional)]),
     [ft_config_exec_prefix="$withval"],
     [ft_config_exec_prefix=""])

   AC_ARG_ENABLE(freetypetest,
     AC_HELP_STRING([--disable-freetypetest],
                    [Do not try to compile and run a test FreeType program]),
     [],
     [enable_fttest=yes])

   if test x$ft_config_exec_prefix != x ; then
     ft_config_args="$ft_config_args --exec-prefix=$ft_config_exec_prefix"
     if test x${FT2_CONFIG+set} != xset ; then
       FT2_CONFIG=$ft_config_exec_prefix/bin/freetype-config
     fi
   fi

   if test x$ft_config_prefix != x ; then
     ft_config_args="$ft_config_args --prefix=$ft_config_prefix"
     if test x${FT2_CONFIG+set} != xset ; then
       FT2_CONFIG=$ft_config_prefix/bin/freetype-config
     fi
   fi

   AC_PATH_PROG([FT2_CONFIG], [freetype-config], [no])

   min_ft_version=m4_if([$1], [], [7.0.1], [$1])
   AC_MSG_CHECKING([for FreeType -- version >= $min_ft_version])
   no_ft=""
   if test "$FT2_CONFIG" = "no" ; then
     no_ft=yes
   else
     FT2_CFLAGS=`$FT2_CONFIG $ft_config_args --cflags`
     FT2_LIBS=`$FT2_CONFIG $ft_config_args --libs`
     ft_config_major_version=`$FT2_CONFIG $ft_config_args --version | \
       sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
     ft_config_minor_version=`$FT2_CONFIG $ft_config_args --version | \
       sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
     ft_config_micro_version=`$FT2_CONFIG $ft_config_args --version | \
       sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
     ft_min_major_version=`echo $min_ft_version | \
       sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
     ft_min_minor_version=`echo $min_ft_version | \
       sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
     ft_min_micro_version=`echo $min_ft_version | \
       sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
     if test x$enable_fttest = xyes ; then
       ft_config_is_lt=""
       if test $ft_config_major_version -lt $ft_min_major_version ; then
         ft_config_is_lt=yes
       else
         if test $ft_config_major_version -eq $ft_min_major_version ; then
           if test $ft_config_minor_version -lt $ft_min_minor_version ; then
             ft_config_is_lt=yes
           else
             if test $ft_config_minor_version -eq $ft_min_minor_version ; then
               if test $ft_config_micro_version -lt $ft_min_micro_version ; then
                 ft_config_is_lt=yes
               fi
             fi
           fi
         fi
       fi
       if test x$ft_config_is_lt = xyes ; then
         no_ft=yes
       else
         ac_save_CFLAGS="$CFLAGS"
         ac_save_LIBS="$LIBS"
         CFLAGS="$CFLAGS $FT2_CFLAGS"
         LIBS="$FT2_LIBS $LIBS"

         #
         # Sanity checks for the results of freetype-config to some extent.
         #
         AC_RUN_IFELSE([
             AC_LANG_SOURCE([[

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>

int
main()
{
  FT_Library library;
  FT_Error  error;

  error = FT_Init_FreeType(&library);

  if (error)
    return 1;
  else
  {
    FT_Done_FreeType(library);
    return 0;
  }
}

             ]])
           ],
           [],
           [no_ft=yes],
           [echo $ECHO_N "cross compiling; assuming OK... $ECHO_C"])

         CFLAGS="$ac_save_CFLAGS"
         LIBS="$ac_save_LIBS"
       fi             # test $ft_config_version -lt $ft_min_version
     fi               # test x$enable_fttest = xyes
   fi                 # test "$FT2_CONFIG" = "no"

   if test x$no_ft = x ; then
     AC_MSG_RESULT([yes])
     m4_if([$2], [], [:], [$2])
   else
     AC_MSG_RESULT([no])
     if test "$FT2_CONFIG" = "no" ; then
       AC_MSG_WARN([

  The freetype-config script installed by FreeType 2 could not be found.
  If FreeType 2 was installed in PREFIX, make sure PREFIX/bin is in
  your path, or set the FT2_CONFIG environment variable to the
  full path to freetype-config.
       ])
     else
       if test x$ft_config_is_lt = xyes ; then
         AC_MSG_WARN([

  Your installed version of the FreeType 2 library is too old.
  If you have different versions of FreeType 2, make sure that
  correct values for --with-ft-prefix or --with-ft-exec-prefix
  are used, or set the FT2_CONFIG environment variable to the
  full path to freetype-config.
         ])
       else
         AC_MSG_WARN([

  The FreeType test program failed to run.  If your system uses
  shared libraries and they are installed outside the normal
  system library path, make sure the variable LD_LIBRARY_PATH
  (or whatever is appropiate for your system) is correctly set.
         ])
       fi
     fi

     FT2_CFLAGS=""
     FT2_LIBS=""
     m4_if([$3], [], [:], [$3])
   fi

   AC_SUBST([FT2_CFLAGS])
   AC_SUBST([FT2_LIBS])])

# end of freetype2.m4
