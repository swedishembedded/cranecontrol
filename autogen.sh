#!/bin/sh

set -e

aclocal
automake --add-missing
libtoolize
autoreconf .
