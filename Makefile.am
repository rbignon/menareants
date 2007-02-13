INSTALL_SERVER=@INSTALL_SERVER@
INSTALL_GAME=@INSTALL_GAME@
INSTALL_META_SERVER=@INSTALL_META_SERVER@
SUBDIRS = lib ${INSTALL_META_SERVER} ${INSTALL_SERVER} ${INSTALL_GAME}

AUTOMAKE_OPTIONS = foreign

doxygen:
	@cd src && /usr/bin/doxygen && cd ../server && /usr/bin/doxygen && cd ../lib && /usr/bin/doxygen && cd ../meta-server && /usr/bin/doxygen && cd ../
	@echo "Created code documentation in `pwd`/doc"

update:
	@echo "Updating.."
	@svn update
	@echo "Compilation.."
	@./config.status --recheck > /dev/null > /dev/null
	@make
	@echo "MenAreAnts is Up-to-date !"
	@echo "You have to reinstall (with make install) this game."

clean-generic:
	-rm -f .#* *~

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath
