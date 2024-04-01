#!/bin/bash

export PKG_CONFIG_PATH=`pwd`/usr/lib/pkgconfig

# gnu efi's pkgconfig file doesn't add -L${libdir}
export LIBRARY_PATH=`pwd`/usr/lib/

meson setup build
meson compile -C build --verbose
