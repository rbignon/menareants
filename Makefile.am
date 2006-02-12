INSTALL_SERVER=@INSTALL_SERVER@
INSTALL_GAME=@INSTALL_GAME@
SUBDIRS = lib ${INSTALL_GAME} ${INSTALL_SERVER}

AUTOMAKE_OPTIONS = foreign

doxygen:
	@cd src && /usr/bin/doxygen && cd ../server && /usr/bin/doxygen && cd ../lib && /usr/bin/doxygen && cd ..
	@echo "Created code documentation in `pwd`/doc"
	
clean-generic:
	-rm -f .#* *~