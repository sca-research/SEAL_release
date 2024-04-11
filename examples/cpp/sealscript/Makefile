TARGET=testscript
SOURCE=testscript.cpp

ALLTARGETS=testscript

LDPATH=${HOME}/.local/lib/
#LDPATH=.
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lsealsym -lsealemulator -lseal -lsealscript
#LDLIB=-lemulator
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}
TESTTRACEFILE="/tmp/sealtesttrace.stc"

all: ${ALLTARGETS}

testscript: testscript.cpp
	${CXX} -o $@ $< ${LDLIB}


clean:
	rm -f ${ALLTARGETS} *~
