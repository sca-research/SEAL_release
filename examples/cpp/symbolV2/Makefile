TARGET=encodesym
SOURCE=encodesym.cpp

LDPATH=${HOME}/.local/lib/
#LDPATH=.
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lsealemulator -lseal -lsealsym
#LDLIB=-lemulator
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}
TESTTRACEFILE="/tmp/sealtesttrace.stc"

all: ${TARGET}

encodesym: encodesym.cpp
	${CXX} -o $@ $^ ${LDLIB}



valgrind: ${TARGET}
	valgrind -s --leak-check=full ./encodesym ${TESTTRACEFILE}

clean:
	rm -f ${TARGET} *~
