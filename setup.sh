#!/bin/bash -x

CURRENT=${PWD}
BASEDIR=$(dirname $(readlink -f ${0}))
LIBDIR="${HOME}/.local/lib"
HEADERDIR="${HOME}/.local/include"
LOCALVAR="CURRENT BASEDIR LIBDIR HEADERDIR"

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
ln -fs ${BASEDIR}/sealpy/src/seal.py ${LIBDIR}/seal.py

# Create symbolic links for SEAL library.
ln -fs ${BASEDIR}/src/*.h ${HEADERDIR}/seal/
ln -fs ${BASEDIR}/src/emulator.so ${LIBDIR}/libsealemulator.so
ln -fs ${BASEDIR}/src/seal.so ${LIBDIR}/libseal.so
ln -fs ${BASEDIR}/src/sealsym.so ${LIBDIR}/libsealsym.so
ln -fs ${BASEDIR}/src/sealscript.so ${LIBDIR}/libsealscript.so

# Set environmental variables.
export PYTHONPATH+=":${LIBDIR}:"
export CPATH+=":${HEADERDIR}:"
export LD_LIBRARY_PATH+=":${LIBDIR}:"

# Return to previous folder.
cd ${CURRENT}

# Clean local variables.
for i in ${LOCALVAR}; do
	unset ${i}
done
unset ${LOCALVAR}
