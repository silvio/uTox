#!/bin/sh

set -eux

. ./extra/gitlab/env.sh

# install libsodium, needed for crypto
if ! [ -d libsodium ]; then
  git clone --depth=1 --branch=stable https://github.com/jedisct1/libsodium.git
fi
cd libsodium
git rev-parse HEAD > libsodium.sha
if ! ([ -f "$CACHE_DIR/libsodium.sha" ] && diff "$CACHE_DIR/libsodium.sha" libsodium.sha); then
  ./autogen.sh
  ./configure --prefix="$CACHE_DIR/usr"
  make -j`nproc`
  make install
  mv libsodium.sha "$CACHE_DIR/libsodium.sha"
fi
cd ..
rm -rf libsodium

# install libopus, needed for audio encoding/decoding
if ! [ -f $CACHE_DIR/usr/lib/pkgconfig/opus.pc ]; then
  curl http://downloads.xiph.org/releases/opus/opus-1.1.4.tar.gz -o opus.tar.gz
  tar xzf opus.tar.gz
  cd opus-1.1.4
  ./configure --prefix=$CACHE_DIR/usr
  make -j`nproc`
  make install
  cd ..
  rm -rf opus**
fi

# install libvpx, needed for video encoding/decoding
if ! [ -d libvpx ]; then
  git clone --depth=1 --branch=v1.6.0 https://chromium.googlesource.com/webm/libvpx
fi
cd libvpx
git rev-parse HEAD > libvpx.sha
if ! ([ -f "$CACHE_DIR/libvpx.sha" ] && diff "$CACHE_DIR/libvpx.sha" libvpx.sha); then
  ./configure --prefix=$CACHE_DIR/usr --enable-shared
  make -j`nproc`
  make install
  mv libvpx.sha "$CACHE_DIR/libvpx.sha"
fi
cd ..
rm -rf libvpx

# install toxcore
if ! [ -d toxcore ]; then
  git clone --depth=1 --branch=master https://github.com/$TOXCORE_REPO.git toxcore
fi
cd toxcore
git rev-parse HEAD > toxcore.sha
if ! ([ -f "$CACHE_DIR/toxcore.sha" ] && diff "$CACHE_DIR/toxcore.sha" toxcore.sha); then
  if [ -f CMakeFiles.txt ]; then
    cmake -B_build -H. -DCMAKE_INSTALL_PREFIX:PATH=$CACHE_DIR/usr
  else
    mkdir _build
    autoreconf -fi
    (cd _build && ../configure --prefix=$CACHE_DIR/usr)
  fi
  make -C_build -j`nproc`
  make -C_build install
  mv toxcore.sha "$CACHE_DIR/toxcore.sha"
fi
cd ..
rm -rf toxcore
