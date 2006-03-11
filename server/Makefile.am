SUBDIRS = maps

DEFS = -DPKGDATADIR=\"$(pkgdatadir)/\" @DEFS@
INCLUDES = -I. -I$(top_srcdir) -I$(top_srcdir)/lib
LIBS =

bin_PROGRAMS = euroconqserver
euroconqserver_SOURCES = \
	Main.cpp \
	Server.cpp \
	Config.cpp \
	Debug.cpp \
	Login.cpp \
	Channels.cpp \
	Map.cpp \
	InGame.cpp

euroconqserver_LDADD =$(top_srcdir)/lib/libeuroconq.a

install-data-local:
	$(INSTALL_DATA) euroconqd.conf /etc/euroconqd.conf

clean-generic:
	-rm -f .#* *~
