AC_PREREQ(2.59)
AC_INIT(Makefile.am)

AM_CONFIG_HEADER(config.h)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(euroconq,1)

# Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
#AC_CHECK_LIB(z,main,,echo "Europe Conquest requires zlib to run."; exit 1)
# AC_CHECK_LIB(Hermes,main,,echo "Europe Conquest requires Hermes to run."; exit 1)
#AC_CHECK_LIB(clanCore,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
#AC_CHECK_LIB(clanApp,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
#AC_CHECK_LIB(clanDisplay,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
#AC_CHECK_LIB(clanNetwork,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
# AC_CHECK_LIB(clanSound,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
#AC_CHECK_LIB(png,main,,echo "Europe Conquest requires PNG library to run."; exit 1)
#AC_CHECK_LIB(clanPNG,main,,echo "Europe Conquest requires ClanLib PNG support to run."; exit1)
# AC_CHECK_LIB(clanMikMod,main,
# [
# 	AC_DEFINE(MUSIC, 1, [Define to 1 if you want musc support])
# 	LIBS=$LIBS" -lclanMikMod"
# ],)

# SDL

SDL_VERSION=1.2.6

AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

# SLD_image

AC_CHECK_LIB(SDL_image, IMG_Load,,
        AC_MSG_ERROR([SDL_image library required]))
LIBS="$LIBS -lSDL_image"

# SDL_ttf

AC_CHECK_LIB([SDL_ttf], [TTF_OpenFont],,
        AC_MSG_ERROR([SDL_ttf library is required]))
LIBS="$LIBS -lSDL_ttf"

# SDL_gfx

AC_CHECK_LIB(SDL_gfx, rotozoomSurfaceXY,,
        AC_MSG_ERROR([SDL_gfx library >= 2.0.13 required]))
LIBS="$LIBS -lSDL_gfx"

#  WARNINGS
AC_ARG_ENABLE(  warnings,
                [AC_HELP_STRING([--enable-warnings],[enable warnings during compilation])],
                [
                        if test "x${enableval}" != "xno"; then
                                CXXFLAGS="$CXXFLAGS -Wall"
                        fi
                ],
                )

# COMPILER ET INSTALLER LE SERVEUR

AC_ARG_ENABLE(  server,
                [AC_HELP_STRING([--enable-server],[compilation of Europe Conquest's server])],
                [
                        if test "x${enableval}" != "xno"; then
                                INSTALL_SERVER=server
                                AC_SUBST(INSTALL_SERVER)
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


AC_CONFIG_FILES(Makefile src/Makefile src/data/Makefile lib/Makefile server/Makefile)

AC_OUTPUT
