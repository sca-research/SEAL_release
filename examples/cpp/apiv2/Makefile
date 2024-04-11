TARGET=gentrace
SOURCE=gentrace.cpp

ALLTARGETS=gentrace fifo reader

LDPATH=${HOME}/.local/lib/
#LDPATH=.
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lsealsym -lsealemulator -lseal
#LDLIB=-lemulator
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}
TESTTRACEFILE="/tmp/sealtesttrace.stc"

all: ${ALLTARGETS}

gentrace: gentrace.cpp
	${CXX} -o $@ $< ${LDLIB}

fifo: gentracefifo.cpp
	${CXX} -o $@ $< ${LDLIB}

reader: reader.cpp
	${CXX} -o $@ $< ${LDLIB}

valgrind: ${TARGET}
	valgrind -s --leak-check=full gentrace ${TESTTRACEFILE}

clean:
	rm -f ${ALLTARGETS} *~
