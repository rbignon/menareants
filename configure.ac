AC_PREREQ(2.59)
AC_INIT(Makefile.am)

AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE(euroconq,1)

# Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB(z,main,,echo "Europe Conquest requires zlib to run."; exit 1)
# AC_CHECK_LIB(Hermes,main,,echo "Europe Conquest requires Hermes to run."; exit 1)
AC_CHECK_LIB(clanCore,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
AC_CHECK_LIB(clanApp,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
AC_CHECK_LIB(clanDisplay,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
AC_CHECK_LIB(clanNetwork,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
# AC_CHECK_LIB(clanSound,main,,echo "Europe Conquest requires ClanLib to run."; exit 1)
AC_CHECK_LIB(png,main,,echo "Europe Conquest requires PNG library to run."; exit 1)
AC_CHECK_LIB(clanPNG,main,,echo "Europe Conquest requires ClanLib PNG support to run."; exit1)
# AC_CHECK_LIB(clanMikMod,main,
# [
# 	AC_DEFINE(MUSIC, 1, [Define to 1 if you want musc support])
# 	LIBS=$LIBS" -lclanMikMod"
# ],)

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

AC_OUTPUT(Makefile src/Makefile src/pics/Makefile lib/Makefile server/Makefile)
