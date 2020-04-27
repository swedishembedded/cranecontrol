#!/bin/sh

automake --add-missing
libtoolize
autoreconf .
