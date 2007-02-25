SUBDIRS = maps

DEFS = -DCONFDIR=\"${sysconfdir}/\" -DPKGDATADIR=\"$(pkgdatadir)/\" @DEFS@
INCLUDES = -I$(top_srcdir)/lib
LIBS =

man6_MANS = menareants-server.6
EXTRA_DIST = $(man6_MANS)

bin_PROGRAMS = menareants-server
menareants_server_SOURCES = \
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

menareants_server_LDADD =$(top_srcdir)/lib/libmenareants.a

install-data-local:
	@if test ! -f "$(DESTDIR)/${sysconfdir}/menareantsd.conf"; then \
		if test ! -d "$(DESTDIR)/${sysconfdir}"; then mkdir -p $(DESTDIR)/${sysconfdir}; fi; \
		echo "Installation de la configuration dans $(DESTDIR)/${sysconfdir}"; \
	   $(INSTALL_DATA) menareantsd.conf $(DESTDIR)/${sysconfdir}/menareantsd.conf; \
	fi;

clean-generic:
	-rm -f .#* *~