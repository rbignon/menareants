#! /bin/sh

# R�g�n�re les fichiers autoconf / automake

rm -f config.cache
rm -f config.log
aclocal
autoconf
autoheader
automake -a -c
echo "Now run ./configure"
