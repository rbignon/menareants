SUBDIRS = maps

DEFS = -DCONFDIR=\"${sysconfdir}/\" -DPKGDATADIR=\"$(pkgdatadir)/\" @DEFS@
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
	IA.cpp \
	Map.cpp \
	Batiments.cpp \
	Units.cpp \
	InGame.cpp

menareantsserver_LDADD =$(top_srcdir)/lib/libmenareants.a

install-data-local:
	@if test ! -f "${sysconfdir}/menareantsd.conf"; then \
		if test ! -d "${sysconfdir}"; then mkdir ${sysconfdir}; fi; \
		echo "Installation de la configuration dans ${sysconfdir}"; \
	   $(INSTALL_DATA) menareantsd.conf ${sysconfdir}/menareantsd.conf; \
	fi;

clean-generic:
	-rm -f .#* *~