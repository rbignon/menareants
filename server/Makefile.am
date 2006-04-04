SUBDIRS = maps

DEFS = -DPKGDATADIR=\"$(pkgdatadir)/\" @DEFS@
INCLUDES = -I. -I$(top_srcdir) -I$(top_srcdir)/lib
LIBS =

bin_PROGRAMS = menareantsserver
menareantsserver_SOURCES = \
	Main.cpp \
	Server.cpp \
	Config.cpp \
	Debug.cpp \
	Login.cpp \
	Channels.cpp \
	Map.cpp \
	Units.cpp \
	InGame.cpp

menareantsserver_LDADD =$(top_srcdir)/lib/libmenareants.a

install-data-local:
	$(INSTALL_DATA) menareantsd.conf /etc/menareantsd.conf

clean-generic:
	-rm -f .#* *~
