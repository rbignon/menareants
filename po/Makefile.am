# Example for use of GNU gettext.
# Copyright (C) 2003 Free Software Foundation, Inc.
# This file is in the public domain.
#
# Makefile configuration - processed by automake.
#

#MAINTAINERCLEANFILES = Makefile.in
MAINTAINERCLEANFILES =

POTFILES = \
		src/tools/Font.cpp \
		src/tools/Color.cpp \
		src/tools/Images.cpp \
		src/tools/Maths.cpp \
		src/tools/Video.cpp \
		src/tools/SDL_rotozoom.c \
		src/tools/SDL_gfxPrimitives.c \
		src/gui/Cursor.cpp \
		src/gui/Component.cpp \
		src/gui/Form.cpp \
		src/gui/ChildForm.cpp \
		src/gui/ListBox.cpp \
		src/gui/ComboBox.cpp \
		src/gui/CheckBox.cpp \
		src/gui/Boutton.cpp \
		src/gui/BouttonText.cpp \
		src/gui/Memo.cpp \
		src/gui/Edit.cpp \
		src/gui/MessageBox.cpp \
		src/gui/Label.cpp \
		src/gui/SpinEdit.cpp \
		src/gui/ColorEdit.cpp \
		src/gui/Image.cpp \
		src/gui/ShowMap.cpp \
		src/gui/ProgressBar.cpp \
		src/gui/Fps.cpp \
		src/Main.cpp \
		src/Sockets.cpp \
		src/Config.cpp \
		src/Login.cpp \
		src/Channels.cpp \
		src/Map.cpp \
		src/Batiments.h \
		src/Batiments.cpp \
		src/Units.h \
		src/Units.cpp \
		src/MapEditor.cpp \
		src/JoinGame.cpp \
		src/InGame.cpp

DOMAIN = $(PACKAGE)

COPYRIGHT_HOLDER = CoderZ

MSGID_BUGS_ADDRESS = progs@coderz.info

localedir=@LOCALEDIR@

XGETTEXT = xgettext
MSGMERGE = msgmerge
MSGMERGE_UPDATE = @MSGMERGE@ --quiet --update
MSGINIT = msginit
MSGCONV = msgconv
MSGFILTER = msgfilter

# This is computed as $(foreach file, $(POTFILES), $(top_srcdir)/$(file))
POTFILES_DEPS = @POTFILES_DEPS@

# This is computed as $(foreach lang, $(LINGUAS), $(srcdir)/$(lang).po)
POFILES = @POFILES@
# This is computed as $(foreach lang, $(LINGUAS), $(srcdir)/$(lang).gmo)
GMOFILES = @GMOFILES@
# This is computed as $(foreach lang, $(LINGUAS), $(lang).po-update)
UPDATEPOFILES = @UPDATEPOFILES@
# This is computed as $(foreach lang, $(LINGUAS), $(lang).nop)
DUMMYPOFILES = @DUMMYPOFILES@

# This is computed as
# $(foreach lang, user-specified subset of $(LINGUAS), $(lang).gmo)
CATALOGS = @CATALOGS@

SUFFIXES = .po .gmo .mo .sed .sin .nop .po-update

.po.gmo:
	@lang=`echo $* | sed -e 's,.*/,,'`; \
	test "$(srcdir)" = . && cdcmd="" || cdcmd="cd $(srcdir) && "; \
	echo "Create $${lang}.gmo"; \
	cd $(srcdir) && rm -f $${lang}.gmo && $(GMSGFMT) -c -o t-$${lang}.gmo $${lang}.po && mv t-$${lang}.gmo $${lang}.gmo

.sin.sed:
	@echo "Create $@ file"
	@sed -e '/^#/d' $< > t-$@
	@mv t-$@ $@


all-local: all-local-@USE_NLS@

all-local-yes: $(GMOFILES)
all-local-no:

# stamp-po is a timestamp denoting the last time at which the CATALOGS have
# been loosely updated. Its purpose is that when a developer or translator
# checks out the package via CVS, and the $(DOMAIN).pot file is not in CVS,
# "make" will update the $(DOMAIN).pot and the $(CATALOGS), but subsequent
# invocations of "make" will do nothing. This timestamp would not be necessary
# if updating the $(CATALOGS) would always touch them; however, the rule for
# $(POFILES) has been designed to not touch files that don't need to be
# changed.
stamp-po: $(srcdir)/$(DOMAIN).pot
	test -z "$(GMOFILES)" || $(MAKE) $(GMOFILES)
	@echo "touch stamp-po"
	@echo timestamp > stamp-poT
	@mv stamp-poT stamp-po

# Note: Target 'all' must not depend on target '$(DOMAIN).pot-update',
# otherwise packages like GCC can not be built if only parts of the source
# have been downloaded.

# This target rebuilds $(DOMAIN).pot; it is an expensive operation.
# Note that $(DOMAIN).pot is not touched if it doesn't need to be changed.
$(DOMAIN).pot-update: $(POTFILES_DEPS)
	@echo "Create $(DOMAIN).pot"
	@$(XGETTEXT) \
	  -o $(DOMAIN).pot -k'_' -s --from-code utf-8 \
	  --default-domain=$(DOMAIN) --directory=$(top_srcdir) \
	  --add-comments=TRANSLATORS: $(XGETTEXT_OPTIONS) \
	  --copyright-holder='$(COPYRIGHT_HOLDER)' \
	  --msgid-bugs-address='$(MSGID_BUGS_ADDRESS)' \
	  $(POTFILES)
	@test ! -f $(DOMAIN).po || { \
	  if test -f $(srcdir)/$(DOMAIN).pot; then \
	    sed -e '/^"POT-Creation-Date:/D' < $(srcdir)/$(DOMAIN).pot > $(DOMAIN).1po && \
	    sed -e '/^"POT-Creation-Date:/D' < $(DOMAIN).po > $(DOMAIN).2po && \
	    if cmp $(DOMAIN).1po $(DOMAIN).2po >/dev/null 2>&1; then \
	      rm -f $(DOMAIN).1po $(DOMAIN).2po $(DOMAIN).po; \
	    else \
	      rm -f $(DOMAIN).1po $(DOMAIN).2po $(srcdir)/$(DOMAIN).pot && \
	      mv $(DOMAIN).po $(srcdir)/$(DOMAIN).pot; \
	    fi; \
	  else \
	    mv $(DOMAIN).po $(srcdir)/$(DOMAIN).pot; \
	  fi; \
	}

# This rule has no dependencies: we don't need to update $(DOMAIN).pot at
# every "make" invocation, only create it when it is missing.
# Only "make $(DOMAIN).pot-update" or "make dist" will force an update.
$(srcdir)/$(DOMAIN).pot:
	$(MAKE) $(DOMAIN).pot-update

# This target rebuilds a PO file if $(DOMAIN).pot has changed.
# Note that a PO file is not touched if it doesn't need to be changed.
$(POFILES): $(srcdir)/$(DOMAIN).pot
	@lang=`echo $@ | sed -e 's,.*/,,' -e 's/\.po$$//'`; \
	test "$(srcdir)" = . && cdcmd="" || cdcmd="cd $(srcdir) && "; \
	echo "Update $${lang}.po"; \
	cd $(srcdir) && $(MSGMERGE_UPDATE) $${lang}.po $(DOMAIN).pot && \
	sed -e '/^"POT-Creation-Date:/D' <$${lang}.po >t-$${lang}.po && \
	mv -f t-$${lang}.po $${lang}.po


install-data-local: install-data-local-@USE_NLS@
install-data-local-no: all-local
install-data-local-yes: all-local
	$(mkinstalldirs) $(DESTDIR)$(datadir)
	for cat in $(CATALOGS); do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed -e 's/\.gmo$$//'`; \
	  dir=$(localedir)/$$lang/LC_MESSAGES; \
	  $(mkinstalldirs) $(DESTDIR)$$dir; \
	  if test -r $$cat; then realcat=$$cat; else realcat=$(srcdir)/$$cat; fi; \
	  $(INSTALL_DATA) $$realcat $(DESTDIR)$$dir/$(DOMAIN).mo; \
	  echo "installing $$realcat as $(DESTDIR)$$dir/$(DOMAIN).mo"; \
	  for lc in '' $(EXTRA_LOCALE_CATEGORIES); do \
	    if test -n "$$lc"; then \
	      if (cd $(DESTDIR)$(localedir)/$$lang && LC_ALL=C ls -l -d $$lc 2>/dev/null) | grep ' -> ' >/dev/null; then \
	        link=`cd $(DESTDIR)$(localedir)/$$lang && LC_ALL=C ls -l -d $$lc | sed -e 's/^.* -> //'`; \
	        mv $(DESTDIR)$(localedir)/$$lang/$$lc $(DESTDIR)$(localedir)/$$lang/$$lc.old; \
	        mkdir $(DESTDIR)$(localedir)/$$lang/$$lc; \
	        (cd $(DESTDIR)$(localedir)/$$lang/$$lc.old && \
	         for file in *; do \
	           if test -f $$file; then \
	             ln -s ../$$link/$$file $(DESTDIR)$(localedir)/$$lang/$$lc/$$file; \
	           fi; \
	         done); \
	        rm -f $(DESTDIR)$(localedir)/$$lang/$$lc.old; \
	      else \
	        if test -d $(DESTDIR)$(localedir)/$$lang/$$lc; then \
	          :; \
	        else \
	          rm -f $(DESTDIR)$(localedir)/$$lang/$$lc; \
	          mkdir $(DESTDIR)$(localedir)/$$lang/$$lc; \
	        fi; \
	      fi; \
	      rm -f $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo; \
	      ln -s ../LC_MESSAGES/$(DOMAIN).mo $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo 2>/dev/null || \
	      ln $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(DOMAIN).mo $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo 2>/dev/null || \
	      cp -p $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(DOMAIN).mo $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo; \
	      echo "installing $$realcat link as $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo"; \
	    fi; \
	  done; \
	done

installdirs-local: installdirs-local-@USE_NLS@
installdirs-local-no:
installdirs-local-yes:
	$(mkinstalldirs) $(DESTDIR)$(datadir)
	@catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed -e 's/\.gmo$$//'`; \
	  dir=$(localedir)/$$lang/LC_MESSAGES; \
	  $(mkinstalldirs) $(DESTDIR)$$dir; \
	  for lc in '' $(EXTRA_LOCALE_CATEGORIES); do \
	    if test -n "$$lc"; then \
	      if (cd $(DESTDIR)$(localedir)/$$lang && LC_ALL=C ls -l -d $$lc 2>/dev/null) | grep ' -> ' >/dev/null; then \
	        link=`cd $(DESTDIR)$(localedir)/$$lang && LC_ALL=C ls -l -d $$lc | sed -e 's/^.* -> //'`; \
	        mv $(DESTDIR)$(localedir)/$$lang/$$lc $(DESTDIR)$(localedir)/$$lang/$$lc.old; \
	        mkdir $(DESTDIR)$(localedir)/$$lang/$$lc; \
	        (cd $(DESTDIR)$(localedir)/$$lang/$$lc.old && \
	         for file in *; do \
	           if test -f $$file; then \
	             ln -s ../$$link/$$file $(DESTDIR)$(localedir)/$$lang/$$lc/$$file; \
	           fi; \
	         done); \
	        rm -f $(DESTDIR)$(localedir)/$$lang/$$lc.old; \
	      else \
	        if test -d $(DESTDIR)$(localedir)/$$lang/$$lc; then \
	          :; \
	        else \
	          rm -f $(DESTDIR)$(localedir)/$$lang/$$lc; \
	          mkdir $(DESTDIR)$(localedir)/$$lang/$$lc; \
	        fi; \
	      fi; \
	    fi; \
	  done; \
	done

uninstall-local: uninstall-local-@USE_NLS@
uninstall-local-no:
uninstall-local-yes:
	catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed -e 's/\.gmo$$//'`; \
	  for lc in LC_MESSAGES $(EXTRA_LOCALE_CATEGORIES); do \
	    rm -f $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo; \
	  done; \
	done

html ID:

MOSTLYCLEANFILES =
MOSTLYCLEANFILES += stamp-poT
MOSTLYCLEANFILES += core core.* $(DOMAIN).po $(DOMAIN).1po $(DOMAIN).2po *.new.po
MOSTLYCLEANFILES += *.o *.gmo $(srcdir)/$(DOMAIN).pot *.*~

DISTCLEANFILES = *.mo *.gmo

MAINTAINERCLEANFILES += stamp-po $(GMOFILES)

EXTRA_DIST = LINGUAS $(DOMAIN).pot stamp-po $(POFILES)

# Hidden from automake, but really activated. Works around an automake-1.5 bug.
#distdir: distdir1
distdir1:
	$(MAKE) update-po

update-po: Makefile
	$(MAKE) $(DOMAIN).pot-update
	test -z "$(UPDATEPOFILES)" || $(MAKE) $(UPDATEPOFILES)
	$(MAKE) update-gmo

# General rule for updating PO files.

.nop.po-update:
	@lang=`echo $@ | sed -e 's/\.po-update$$//'`; \
	tmpdir=`pwd`; \
	echo "$$lang:"; \
	test "$(srcdir)" = . && cdcmd="" || cdcmd="cd $(srcdir) && "; \
	echo "$${cdcmd}$(MSGMERGE) $$lang.po $(DOMAIN).pot -o $$lang.new.po"; \
	cd $(srcdir); \
	if $(MSGMERGE) $$lang.po $(DOMAIN).pot -o $$tmpdir/$$lang.new.po; then \
	  if cmp $$lang.po $$tmpdir/$$lang.new.po >/dev/null 2>&1; then \
	    rm -f $$tmpdir/$$lang.new.po; \
	  else \
	    if mv -f $$tmpdir/$$lang.new.po $$lang.po; then \
	      :; \
	    else \
	      echo "msgmerge for $$lang.po failed: cannot move $$tmpdir/$$lang.new.po to $$lang.po" 1>&2; \
	      exit 1; \
	    fi; \
	  fi; \
	else \
	  echo "msgmerge for $$lang.po failed!" 1>&2; \
	  rm -f $$tmpdir/$$lang.new.po; \
	fi

$(DUMMYPOFILES):

update-gmo: Makefile $(GMOFILES)
	@:
