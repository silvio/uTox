#!/bin/sh
set -eux

. ./extra/travis/env.sh

export CFLAGS="-I$CACHE_DIR/usr/include -I/usr/share/mingw-w64/include/ "

mkdir build_win
cd build_win
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-win32.cmake -DWIN_XP_LEGACY=ON
make VERBOSE=1
