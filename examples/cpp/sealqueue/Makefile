TARGET=sealqueue
SOURCE=${TARGET}.cpp

ALLTARGETS=${TARGET}

LDPATH=${HOME}/.local/lib/
#LDPATH=.
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lsealsym -lsealemulator -lseal -lsealscript
#LDLIB=-lemulator
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}
TESTTRACEFILE="/tmp/sealtesttrace.stc"

all: ${ALLTARGETS}

${TARGET}: ${SOURCE}
	${CXX} -o $@ $< ${LDLIB}


clean:
	rm -f ${ALLTARGETS} *~
