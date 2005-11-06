DEFS = -DPKGDATADIR=\"$(pkgdatadir)/\" @DEFS@
INCLUDES = -I. -I$(top_srcdir)

bin_PROGRAMS = euroconq
euroconq_SOURCES = Main.cpp Menu.cpp Timer.cpp Resources.cpp Sockets.cpp
euroconq_LDADD   =

SUBDIRS = pics

install-data-local: euroconq.dat
	$(mkinstalldirs) @datadir@/euroconq/
	$(INSTALL_DATA) euroconq.dat @datadir@/euroconq/euroconq.dat
	$(mkinstalldirs) @datadir@/euroconq/
	$(INSTALL_DATA) euroconq.scr @datadir@/euroconq/euroconq.scr

uninstall-local:
	-rm -f @datadir@/euroconq/euroconq.dat
	-rm -f @datadir@/euroconq/euroconq.scr

euroconq.dat: euroconq.scr pics/*.pcx pics/*.png
	@./euroconq -datafile

all-local: euroconq.dat

distclean-local:
	rm -f euroconq.dat euroconq.cfg


.PHONY: files

files:
	@files=^Ls $(DISTFILES) 2> /dev/null ; for p in $$files; do \
		echo $$p; \
	done
	@for subdir in $(SUBDIRS); do \
		files=^Cd $$subdir; $(MAKE) files | grep -v "make\[[1-9]\]"; \
		for file in $$files; do \
		echo $$subdir/$$file; \
		done; \
	done
