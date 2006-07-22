SUBDIRS = data

bin_PROGRAMS = menareants

DEFS = -DPKGDATADIR=\"$(pkgdatadir)/\" @DEFS@
INCLUDES = -I. -I$(top_srcdir) -I${top_srcdir}/lib

EXTRA_DIST = v.sh

DISTCLEANFILES = Version.cpp

nodist_menareants_SOURCES = Version.cpp
menareants_SOURCES = \
		tools/Font.cpp \
		tools/Color.cpp \
		tools/Images.cpp \
		tools/Maths.cpp \
		tools/Video.cpp \
		gui/Cursor.cpp \
		gui/Component.cpp \
		gui/Form.cpp \
		gui/ChildForm.cpp \
		gui/ListBox.cpp \
		gui/ComboBox.cpp \
		gui/CheckBox.cpp \
		gui/Boutton.cpp \
		gui/BouttonText.cpp \
		gui/Memo.cpp \
		gui/Edit.cpp \
		gui/MessageBox.cpp \
		gui/Label.cpp \
		gui/SpinEdit.cpp \
		gui/ColorEdit.cpp \
		gui/Image.cpp \
		gui/ShowMap.cpp \
		gui/ProgressBar.cpp \
		gui/Fps.cpp \
		Main.cpp \
		Timer.cpp \
		Resources.cpp \
		AltThread.cpp \
		Sockets.cpp \
		Config.cpp \
		Debug.cpp \
		Login.cpp \
		Channels.cpp \
		Map.cpp \
		Batiments.cpp \
		Units.cpp \
		MapEditor.cpp \
		JoinGame.cpp \
		InGame.cpp

menareants_LDADD =${top_builddir}/lib/libmenareants.a

Version.cpp: $(menareants_SOURCES)
	@$(SHELL) v.sh

distclean-local:
	rm -f menareants.cfg

clean-generic:
	-rm -f .#* *~
	-rm -f gui/.#* gui/*~
	-rm -f tools/.#* tools/*~

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
