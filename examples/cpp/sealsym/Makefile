TARGET=sealsym
SOURCE=sealsym.cpp

LDPATH=${HOME}/.local/lib/
#LDPATH=.
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lsealemulator -lseal -lsealsym
#LDLIB=-lemulator
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}

all: ${TARGET}

${TARGET}: ${SOURCE}
	${CXX} -o $@ $< ${LDLIB}


valgrind: ${TARGET}
	valgrind -s --leak-check=full ./${TARGET}

clean:
	rm -f ${TARGET} *~
