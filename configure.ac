AC_PREREQ(2.59)

AC_INIT([MenAreAnts], [0.1], [bugs@coderz.info], [menareants])

AM_CONFIG_HEADER(config.h)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([foreign 1.5])

# Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AM_GNU_GETTEXT([external])

dnl ========================================================================
dnl == Set flags for various environments                                  =
dnl ========================================================================
case "${host}" in
  i[[3456789]]86-*-mingw32*) WIN32="yes" ;;
  *cygwin*) WIN32="yes" ;;
  *) WIN32="no" ;;
esac
AM_CONDITIONAL([WIN32], test "$WIN32" = "yes")

AM_CONDITIONAL([INCLUDEDINTL], [test x$nls_cv_use_gnu_gettext = xyes])

if test "x${prefix}" = "xNONE"; then
  prefix="${ac_default_prefix}"
fi

# COMPILER ET INSTALLER LE SERVEUR

AC_ARG_ENABLE(  server,
                [AC_HELP_STRING([--enable-server],[compilation of Men Are Ants' server])],
                [
                        if test "x${enableval}" != "xno"; then
                                INSTALL_SERVER=server
                                AC_SUBST(INSTALL_SERVER)
                        fi
                ],
        )

AC_ARG_ENABLE(  meta-server,
                [AC_HELP_STRING([--enable-meta-server],[compilation of Men Are Ants' meta-server])],
                [
                        if test "x${enableval}" != "xno"; then
                                INSTALL_META_SERVER=meta-server
                                AC_SUBST(INSTALL_META_SERVER)
                        fi
                ],
        )

# NE PAS COMPILER ET INSTALLER LE JEU

INSTALL_GAME_Q=1

AC_ARG_ENABLE(  game,
                [AC_HELP_STRING([--disable-game],[compilation without Men Are Ants' game])],
                [
                        if test "x${enableval}" = "xno"; then
                                INSTALL_GAME_Q=0
                        fi
                ],
                INSTALL_GAME_Q=1
        )

if test "x${INSTALL_GAME_Q}" = "x1"; then
  INSTALL_GAME="src po"
  AC_SUBST(INSTALL_GAME)
fi

# SDL

SDL_VERSION=1.2.6

AM_PATH_SDL($SDL_VERSION,
			:,
			if test "x{INSTALL_GAME_Q}" = "x1"; then
				AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
			fi
			)

# SLD_image

AC_CHECK_LIB(SDL_image, IMG_Load,,
				if test "x{INSTALL_GAME_Q}" = "x1"; then
					AC_MSG_ERROR([SDL_image library required])
				fi
			)

# SDL_mixer

AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio,,
				if test "x{INSTALL_GAME_Q}" = "x1"; then
					AC_MSG_ERROR([SDL_mixer library required])
				fi
			)

# SDL_ttf

AC_CHECK_LIB([SDL_ttf], [TTF_OpenFont],,
				if test "x{INSTALL_GAME_Q}" = "x1"; then
					AC_MSG_ERROR([SDL_ttf library is required])
				fi)

# SDL_gfx

AC_CHECK_LIB(SDL_gfx, rotozoomSurfaceXY,,
				if test "x{INSTALL_GAME_Q}" = "x1"; then
					AC_MSG_ERROR([SDL_gfx library >= 2.0.13 required])
				fi)

if test "x${INSTALL_GAME_Q}" = "x1"; then
	LIBS="$LIBS $SDL_LIBS"
	CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
	CFLAGS="$CFLAGS $SDL_CFLAGS"
fi

# WIN32
if test "${WIN32}" = "yes"; then
	LIBS="$LIBS -lSDL_gfx -lSDL_ttf -lwsock32 -lSDL_image -lSDL_mixer -lwinmm -lintl"
#	LIBS="$LIBS -lSDL_gfx -lSDL_ttf -lwsock32 -lSDL_image -lSDL_mixer -lwinmm"
fi

#  FULL-WARNINGS
#AC_ARG_ENABLE(  full-warnings,
#                [AC_HELP_STRING([--enable-full-warnings],[enable warnings during compilation (-W -Wall)])],
#                [
#                        if test "x${enableval}" != "xno"; then
#                                CXXFLAGS="$CXXFLAGS -W -Wall"
#                        fi
#                ],
#                )

#  WARNINGS
AC_ARG_ENABLE(  warnings,
                [AC_HELP_STRING([--enable-warnings],[enable warnings during compilation (-Wall)])],
                [
                        if test "x${enableval}" != "xno"; then
                                WARNINGS=1
                        fi
                ],
                )

# GPROF
AC_ARG_ENABLE(  gprof,
                [AC_HELP_STRING([--enable-gprof],[used to have some cpu's informations])],
                [
                        if test "x${enableval}" != "xno"; then
                                CXXFLAGS="$CXXFLAGS -pg"
                        fi
                ],
                )

#  DEBUG
AC_ARG_ENABLE(  debug,
                [AC_HELP_STRING([--enable-debug],[enable debug])],
                [
                        if test "x${enableval}" != "xno"; then
                                DEBUG=1
                        fi
                ],
                [DEBUG=0 ]
        )

if test "x$GCC" = "xyes"; then
    dnl get gcc version
    AC_MSG_CHECKING([gcc version])
            gccver=$($CC -dumpversion)
            gccvermajor=$(echo $gccver | cut -d . -f1)
            gccverminor=$(echo $gccver | cut -d . -f2)
            gccvernum=$(expr $gccvermajor "*" 100 + $gccverminor)
    AC_MSG_RESULT($gccver)

    if test "x${WARNINGS}" = "x1"; then
      dnl Enable all warnings
# en reserve: -Weffc++ (il fait bien mal aux variables non initialisées)
#             -pedantic (il trouve rien à part une répétition de ;; dans SDL_ttf.h)
#             -Wold-style-cast (prohibe les anciens styles de conversions)
      GCC_FLAGS="-Wall"
      CFLAGS="$CFLAGS -std=c99"
      CXXFLAGS="$CXXFLAGS -Wstrict-null-sentinel -ansi"

      dnl Enable *more* warnings
      if test "$gccvernum" -ge "400"; then
                  dnl gcc 4.0 or later
          GCC_FLAGS="$GCC_FLAGS -Wextra"
      else
          GCC_FLAGS="$GCC_FLAGS -W"
      fi

      dnl Skip 'unused parameter' warning
      GCC_FLAGS="$GCC_FLAGS -Wno-unused-parameter"
    fi

    if test "x${DEBUG}" = "x1"; then
        GCC_FLAGS="$GCC_FLAGS -O0 -g -DDEBUG"
    else
        GCC_FLAGS="$GCC_FLAGS -O2"
    fi

    CFLAGS="$CFLAGS $GCC_FLAGS"
    CXXFLAGS="$CXXFLAGS $GCC_FLAGS"
else
    if test "x${DEBUG}" = "x1"; then
        CFLAGS="$CFLAGS -DDEBUG"
        CXXFLAGS="$CXXFLAGS -DDEBUG"
    fi
fi


DATA_FILES=`cd src/data ; find . -name CVS -prune -o -name .cvsignore -o -name "Makefile*" -o -name ".#*" -o -type d -o -print | grep -v .svn`
DATA_FILES=`echo $DATA_FILES`

AC_SUBST([DATA_FILES])

MAP_FILES=`cd server/maps ; find . -name CVS -prune -o -name .cvsignore -o -name "Makefile*" -o -name ".#*" -o -type d -o -print | grep -v .svn`
MAP_FILES=`echo $MAP_FILES`

AC_SUBST([MAP_FILES])

if test "${WIN32}" = "yes"; then LOCALEDIR="locale"
else LOCALEDIR="${prefix}/share/locale"; fi
AC_ARG_WITH([localedir-name],
    [AS_HELP_STRING([--with-localedir-name=DIR],
                    [specify where locales are installed (default: ${prefix}/share/locale)])],
    [LOCALEDIR="$withval"])

AC_SUBST([LOCALEDIR])


AC_CONFIG_FILES([Makefile src/Makefile src/data/Makefile lib/Makefile server/Makefile server/menareantsd.conf server/maps/Makefile meta-server/Makefile po/Makefile], [AM_POSTPROCESS_PO_MAKEFILE])

AC_OUTPUT

