#!/bin/sh

mkdir -p lib
git clone git@gitlab.com:mkschreder/libfirmware.git lib/libfirmware
git clone git@gitlab.com:mkschreder/libdriver.git lib/libdriver
git clone git@gitlab.com:mkschreder/libfdt.git lib/libfdt

ln -s $PWD/lib/libfirmware/src src/libfirmware
ln -s $PWD/lib/libdriver/src src/libdriver
ln -s $PWD/lib/libfdt/src src/libfdt
ln -s $PWD/lib/libfirmware/include include/libfirmware
ln -s $PWD/lib/libdriver/include include/libdriver
ln -s $PWD/lib/libfdt/include include/libfdt

automake --add-missing
libtoolize
autoreconf .
