#!/bin/bash

PREFIX=$PWD/mingw

[ -d $PREFIX ] && rm -rf $PREFIX

mkdir $PREFIX
export PATH=$PREFIX/bin:$PATH CPATH=$PREFIX/include \
       LD_LIBRARY_PATH=$PREFIX/lib LD_RUN_PATH=$PREFIX/lib \
       PKG_CONFIG_LIBDIR=$PREFIX/lib/pkgconfig

cat "PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig pkg-config \$*" > $PREFIX/bin/i586-mingw32msvc-pkg-config
chmod o+x g+rwx $PREFIX/bin/i586-mingw32msvc-pkg-config

# SDL

SDL_VERSION=1.2.14
SDL_FILENAME=SDL-devel-$SDL_VERSION-mingw32.tar.gz
wget http://www.libsdl.org/release/$SDL_FILENAME
tar xvf $SDL_FILENAME
mv SDL-$SDL_VERSION/* $PREFIX/
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

rm $PREFIX/lib/SDL_*.lib $PREFIX/lib/libSDL_*.la
for f in $PREFIX/lib/SDL_*.dll; do var=${f//dll/a}; cp $f ${var//SDL_/libSDL_}; mv $f $PREFIX/bin/; done

# MINGW LIBS
cd $PREFIX
wget http://downloads.sourceforge.net/project/gnuwin32/tiff/3.8.2-1/tiff-3.8.2-1-lib.zip
unzip tiff-3.8.2-1-lib.zip
wget http://downloads.sourceforge.net/project/gnuwin32/zlib/1.2.3/zlib-1.2.3-lib.zip
unzip zlib-1.2.3-lib.zip
wget http://downloads.sourceforge.net/project/gnuwin32/jpeg/6b-4/jpeg-6b-4-lib.zip
unzip jpeg-6b-4-lib.zip
wget http://downloads.sourceforge.net/project/gnuwin32/libpng/1.2.37/libpng-1.2.37-lib.zip
unzip libpng-1.2.37-lib.zip
wget http://downloads.sourceforge.net/project/gnuwin32/libintl/0.14.4/libintl-0.14.4-lib.zip
unzip libintl-0.14.4-lib.zip
rm -f $PREFIX/lib/*.lib $PREFIX/lib/*.la
rm -f *.zip
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

echo ""
echo "Now you can run ./build.sh to build the win32 version of MenAreAnts."
