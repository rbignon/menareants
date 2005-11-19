SUBDIRS = data

DEFS = -DPKGDATADIR=\"$(pkgdatadir)/\" @DEFS@
INCLUDES = -I. -I$(top_srcdir) -I${top_srcdir}/lib

bin_PROGRAMS = euroconq
euroconq_SOURCES = \
		Main.cpp \
		Menu.cpp \
		Timer.cpp \
		Resources.cpp \
		Sockets.cpp \
		Config.cpp \
		Login.cpp \
		tools/Font.cpp \
		tools/Images.cpp

euroconq_LDADD =${top_builddir}/lib/libeuroconq.a

distclean-local:
	rm -f euroconq.cfg


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
