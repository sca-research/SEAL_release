#!/bin/bash

CURRENT=${PWD}
BASEDIR=$(dirname $(readlink -f ${0}))
LIBDIR="/usr/lib/"
PYLIBDIR="/usr/lib/python3/dist-packages"
HEADERDIR="/usr/include/"
LOCALVAR="CURRENT BASEDIR LIBDIR HEADERDIR"

if [ $EUID -ne 0 ]; then
    echo "Installing SEAL requires root privilege."
    exit -1
fi


# Move to base folder.
cd ${BASEDIR}

# Create folders.
if [ ! -d ${LIBDIR} ]; then
	mkdir -p ${LIBDIR}
fi

if [ ! -d ${HEADERDIR} ]; then
	mkdir -p ${HEADERDIR}
fi

mkdir -p "${HEADERDIR}/seal"

# Compile SEAL.
cd src
make rebuild

# Create symbolic links for SEAL Python module.
ln -fs ${BASEDIR}/sealpy/src/seal.py ${PYLIBDIR}/seal.py

# Create symbolic links for SEAL library.
ln -fs ${BASEDIR}/src/*.h ${HEADERDIR}/seal/
ln -fs ${BASEDIR}/src/emulator.so ${LIBDIR}/libsealemulator.so
ln -fs ${BASEDIR}/src/seal.so ${LIBDIR}/libseal.so
ln -fs ${BASEDIR}/src/sealsym.so ${LIBDIR}/libsealsym.so
ln -fs ${BASEDIR}/src/sealscript.so ${LIBDIR}/libsealscript.so

# Return to previous folder.
cd ${CURRENT}

# Clean local variables.
for i in ${LOCALVAR}; do
	unset ${i}
done
unset ${LOCALVAR}
