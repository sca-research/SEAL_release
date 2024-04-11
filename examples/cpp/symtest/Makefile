TARGET=symtest
SOURCE=symtest.cpp

ALLTARGETS=symtest

LDPATH=${HOME}/.local/lib/
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lseal -lsealemulator -lsealsym
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}
TESTTRACEFILE="/tmp/sealtesttrace.stc"

all: ${ALLTARGETS}

symtest: symtest.cpp Makefile
	${CXX} -o $@ $< ${LDLIB}

valgrind: ${TARGET}
	valgrind -s --leak-check=full gentrace ${TESTTRACEFILE}

clean:
	rm -f ${ALLTARGETS} *~
