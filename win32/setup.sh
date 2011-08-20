#!/bin/bash

set -e
set -x

PREFIX=$PWD/mingw

[ -d $PREFIX ] && rm -rf $PREFIX

cp -r /usr/i586-mingw32msvc $PREFIX
export PATH=$PREFIX/bin:$PATH CPATH=$PREFIX/include \
       LD_LIBRARY_PATH=$PREFIX/lib LD_RUN_PATH=$PREFIX/lib \
       PKG_CONFIG_LIBDIR=$PREFIX/lib/pkgconfig

echo "PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig pkg-config \$*" > $PREFIX/bin/i586-mingw32msvc-pkg-config
chmod +x $PREFIX/bin/i586-mingw32msvc-pkg-config

# SDL

SDL_VERSION=1.2.14
SDL_FILENAME=SDL-devel-$SDL_VERSION-mingw32.tar.gz
wget http://www.libsdl.org/release/$SDL_FILENAME
tar xvf $SDL_FILENAME
mv SDL-$SDL_VERSION/bin/* $PREFIX/bin
mv SDL-$SDL_VERSION/include/* $PREFIX/include
mv SDL-$SDL_VERSION/lib/* $PREFIX/lib
rm -rf SDL-*

sed -e "s#^prefix=.*#prefix=$PREFIX#g" $PREFIX/bin/sdl-config > $PREFIX/bin/sdl-config.new
mv $PREFIX/bin/sdl-config.new $PREFIX/bin/sdl-config
chmod +x $PREFIX/bin/sdl-config

rm -f $PREFIX/lib/libSDL.la
#mv SDL.lib libSDL.a
mv $PREFIX/lib/libSDL.dll.a $PREFIX/lib/libSDL.a

SDL_TTF_VERSION=2.0.9
SDL_TTF_FILENAME=SDL_ttf-devel-$SDL_TTF_VERSION-VC8.zip
wget http://www.libsdl.org/projects/SDL_ttf/release/$SDL_TTF_FILENAME
unzip $SDL_TTF_FILENAME
mv SDL_ttf-$SDL_TTF_VERSION/lib/* $PREFIX/lib/
mv SDL_ttf-$SDL_TTF_VERSION/include/* $PREFIX/include/SDL/
rm -rf SDL_ttf-$SDL_TTF_VERSION
rm -f $SDL_TTF_FILENAME

SDL_MIXER_VERSION=1.2.11
SDL_MIXER_FILENAME=SDL_mixer-devel-$SDL_MIXER_VERSION-VC.zip
wget http://www.libsdl.org/projects/SDL_mixer/release/$SDL_MIXER_FILENAME
unzip $SDL_MIXER_FILENAME
mv SDL_mixer-$SDL_MIXER_VERSION/lib/* $PREFIX/lib/
mv SDL_mixer-$SDL_MIXER_VERSION/include/* $PREFIX/include/SDL/
rm -rf SDL_mixer-$SDL_MIXER_VERSION
rm -f $SDL_MIXER_FILENAME

for f in $PREFIX/lib/SDL_*.dll; do var=${f//dll/a}; cp $f ${var//SDL_/libSDL_}; mv $f $PREFIX/bin/; done

# MINGW LIBS
cd $PREFIX
for archive in "tiff-3.8.2-1" "zlib-1.2.3" "jpeg-6b-4" "libpng-1.2.37" "libintl-0.14.4" "libiconv-1.9.2-1"
do
	prj=`echo $archive | cut -d"-" -f1`
	ver=`echo $archive | cut -d"-" -f2-`
	for what in "lib" "bin"
	do
		wget http://downloads.sourceforge.net/project/gnuwin32/$prj/$ver/$prj-$ver-$what.zip
		unzip $prj-$ver-$what.zip
		rm -f $prj-$ver-$what.zip
	done
done
rm -f $PREFIX/lib/*.lib $PREFIX/lib/*.la
cd ..

SDL_IMAGE_VERSION=1.2.10
SDL_IMAGE_FILENAME=SDL_image-$SDL_IMAGE_VERSION.tar.gz
wget http://www.libsdl.org/projects/SDL_image/release/$SDL_IMAGE_FILENAME
tar xvf $SDL_IMAGE_FILENAME
cd SDL_image-$SDL_IMAGE_VERSION/
sh autogen.sh
./configure --target=i586-mingw32msvc --host=i586-mingw32msvc --prefix=$PREFIX \
            --with-sdl-prefix=$PREFIX --enable-shared --disable-static --disable-sdltest
make
make install
cd ../
rm -rf SDL_image-$SDL_IMAGE_VERSION/
rm -rf $SDL_IMAGE_FILENAME

SDL_GFX_VERSION=2.0.20
SDL_GFX_FILENAME=SDL_gfx-$SDL_GFX_VERSION.tar.gz
wget http://www.ferzkopp.net/Software/SDL_gfx-2.0/$SDL_GFX_FILENAME
tar xvf $SDL_GFX_FILENAME
cd SDL_gfx-$SDL_GFX_VERSION/
./configure --target=i586-mingw32msvc --host=i586-mingw32msvc --prefix=$PREFIX \
            --with-sdl-prefix=$PREFIX --enable-mmx --enable-static --disable-shared --disable-sdltest
make
make install
cd ../
rm -rf SDL_gfx-$SDL_GFX_VERSION/
rm -rf $SDL_GFX_FILENAME

set +x

echo ""
echo "Now you can run ./build.sh to build the win32 version of MenAreAnts."
