TARGET=loadcore
SOURCE=loadcore.cpp

LDPATH=${HOME}/.local/lib/
#LDPATH=.
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lsealemulator -lseal
#LDLIB=-lemulator
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}

all: ${TARGET}

${TARGET}: ${SOURCE}
	${CXX} -o ${TARGET} ${SOURCE} ${LDLIB}
clean:
	rm -f ${TARGET}

