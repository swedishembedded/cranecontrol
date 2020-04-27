#!/bin/sh

mkdir -p lib
if [ ! -d lib/libfirmware ]; then
	git clone git@gitlab.com:mkschreder/libfirmware.git lib/libfirmware
fi
if [ ! -d lib/libdriver ]; then
	git clone git@gitlab.com:mkschreder/libdriver.git lib/libdriver
fi
if [ ! -d lib/libfdt ]; then
	git clone git@gitlab.com:mkschreder/libfdt.git lib/libfdt
fi

ln -s $PWD/lib/libfirmware/src src/libfirmware
ln -s $PWD/lib/libdriver/src src/libdriver
ln -s $PWD/lib/libfdt/src src/libfdt
ln -s $PWD/lib/libplc/src src/libplc

ln -s $PWD/lib/libfirmware/include include/libfirmware
ln -s $PWD/lib/libdriver/include include/libdriver
ln -s $PWD/lib/libfdt/include include/libfdt
ln -s $PWD/lib/libplc/include include/libplc

automake --add-missing
libtoolize
autoreconf .
