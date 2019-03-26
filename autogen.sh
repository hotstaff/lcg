#!/bin/sh
autoreconf --install || exit 1
echo "Please run ./configure, make and make install."
