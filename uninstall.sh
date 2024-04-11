#!/bin/bash -x

CURRENT=${PWD}
BASEDIR=$(dirname $(readlink -f ${0}))
LIBDIR="/usr/lib/"
PYLIBDIR="/usr/lib/python3/dist-packages"
HEADERDIR="/usr/include/"
LOCALVAR="CURRENT BASEDIR LIBDIR HEADERDIR"

# Delete headers
ls -l ${HEADERDIR}/*seal*
rm -rf ${HEADERDIR}/seal

# Delete so
ls -l ${LIBDIR}/*seal*
rm -rf ${LIBDIR}/libseal*
