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

dnl ========================================================================
dnl == Set flags for various environments                                  =
dnl ========================================================================
case "${host}" in
  i[[3456789]]86-*-mingw32*) WIN32="yes" ;;
  *cygwin*) WIN32="yes" ;;
  *) WIN32="no" ;;
esac
AM_CONDITIONAL([WIN32], test "$WIN32" = "yes")

# COMPILER ET INSTALLER LE SERVEUR

AC_ARG_ENABLE(  server,
                [AC_HELP_STRING([--enable-server],[compilation of Men Are Ants's server])],
                [
                        if test "x${enableval}" != "xno"; then
                                INSTALL_SERVER=server
                                AC_SUBST(INSTALL_SERVER)
                        fi
                ],
        )

# NE PAS COMPILER ET INSTALLER LE JEU

INSTALL_GAME_Q=1

AC_ARG_ENABLE(  game,
                [AC_HELP_STRING([--disable-game],[compilation of Men Are Ants game])],
                [
                        if test "x${enableval}" = "xno"; then
                                INSTALL_GAME_Q=0
                        fi
                ],
                INSTALL_GAME_Q=1
        )

if test "x${INSTALL_GAME_Q}" = "x1"; then
  INSTALL_GAME=src
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
	LIBS="$LIBS -lSDL_gfx -lSDL_ttf -lwsock32 -lSDL_image -lwinmm"
fi

#  FULL-WARNINGS
AC_ARG_ENABLE(  full-warnings,
                [AC_HELP_STRING([--enable-full-warnings],[enable warnings during compilation (-W -Wall)])],
                [
                        if test "x${enableval}" != "xno"; then
                                CXXFLAGS="$CXXFLAGS -W -Wall"
                        fi
                ],
                )

#  WARNINGS
AC_ARG_ENABLE(  warnings,
                [AC_HELP_STRING([--enable-warnings],[enable warnings during compilation (-Wall)])],
                [
                        if test "x${enableval}" != "xno"; then
                                CXXFLAGS="$CXXFLAGS -Wall"
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
if test "x${DEBUG}" = "x1"; then
        AC_DEFINE_UNQUOTED(DEBUG, 1, [Defined if debug is enabled])
fi

DATA_FILES=`cd src/data ; find . -name CVS -prune -o -name .cvsignore -o -name ".svn*" -o -name "Makefile*" -o -name ".#*" -o -type d -o -print`
DATA_FILES=`echo $DATA_FILES`

AC_SUBST([DATA_FILES])

MAP_FILES=`cd server/maps ; find . -name CVS -prune -o -name .cvsignore -o -name ".svn*" -o -name "Makefile*" -o -name ".#*" -o -type d -o -print`
MAP_FILES=`echo $MAP_FILES`

AC_SUBST([MAP_FILES])

AC_CONFIG_FILES(Makefile src/Makefile src/data/Makefile lib/Makefile server/Makefile server/maps/Makefile)

AC_OUTPUT

