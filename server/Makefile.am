DEFS = -DPKGDATADIR=\"$(pkgdatadir)/\" @DEFS@
INCLUDES = -I. -I$(top_srcdir) -I$(top_srcdir)/lib
LIBS =

bin_PROGRAMS = euroconqserver
euroconqserver_SOURCES = Main.cpp Server.cpp Config.cpp Login.cpp Channels.h
euroconqserver_LDADD =$(top_srcdir)/lib/libeuroconq.a

install-data-local:
	$(INSTALL_DATA) euroconqd.conf /etc/euroconqd.conf
