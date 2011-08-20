#!/bin/bash

PREFIX=$PWD/mingw
SRC=$PWD/maa-src
TARGET=$PWD/MenAreAnts
EXTRA=$PWD/extra

if [ ! -d $PREFIX ]; then
	echo "Please run ./setup.sh before."
	exit 1
fi

set -e
set -x

export PATH=$PREFIX/bin:$PATH CPATH=$PREFIX/include \
       LD_LIBRARY_PATH=$PREFIX/lib LD_RUN_PATH=$PREFIX/lib \
       PKG_CONFIG_LIBDIR=$PREFIX/lib/pkgconfig

[ -d $SRC ] && rm -rf $SRC
mkdir $SRC/
cd ..
cp -r aclocal.m4 config* Make* lib m4 server meta-server po src $SRC
cd $SRC
set +e
make clean || make clean
set -e
./configure --target=i586-mingw32msvc --host=i586-mingw32msvc --enable-debug \
            --with-sdl-prefix=$PREFIX --enable-game --disable-server --disable-meta-server
make

VERSION_MAJOR=`sed -n 's/^#define APP_VERSION_ALPHA\s\"\(.*\)\"/\1/p' < lib/Defines.h`
VERSION_MINOR=`sed -n 's/^#define APP_VERSION_BETA\s\"\(.*\)\"/\1/p' < lib/Defines.h`
TARGET=$TARGET-$VERSION_MAJOR.$VERSION_MINOR

[ -d $TARGET ] && rm -rf $TARGET
mkdir $TARGET
cp -r src/menareants.exe src/data server/maps/*.map $TARGET
cd po/
make update-po >/dev/null
for i in $(ls *.gmo); do
	LANG=`echo $i | sed -e 's/\.gmo//'`;
	mkdir -p $TARGET/locale/$LANG/LC_MESSAGES/;
	cp $i $TARGET/locale/$LANG/LC_MESSAGES/menareants.mo;
done

cd ../
rm -rf $SRC

cp $PREFIX/lib/*.dll $TARGET/
cp $PREFIX/bin/*.dll $TARGET/

if [ -d $EXTRA ]; then
	cp $EXTRA/* $TARGET/
fi

[ -f $TARGET.zip ] && rm -f $TARGET.zip
cd `dirname $TARGET`
zip -r $TARGET.zip `basename $TARGET`

set +x

echo ""
echo "Men Are Ants has been built."
