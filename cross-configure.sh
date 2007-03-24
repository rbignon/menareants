#!/bin/sh

CONFIG_SHELL=/bin/sh
export CONFIG_SHELL
PREFIX=/usr/
TARGET=i586-mingw32msvc
PATH="$PREFIX/$TARGET/bin:$PREFIX/bin:$PATH"
export PATH
if [ -f "$PREFIX/$TARGET/bin/$TARGET-sdl-config" ]; then
    SDL_CONFIG="$PREFIX/$TARGET/bin/$TARGET-sdl-config"
    export SDL_CONFIG
fi
cache=cross-config.cache
sh configure --cache-file="$cache" \
	--target=$TARGET --host=$TARGET --build=i386-linux --with-gnu-ld \
	$*
status=$?
rm -f "$cache"
exit $status
