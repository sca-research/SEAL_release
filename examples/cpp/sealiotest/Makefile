TARGET=sealserver
SOURCE=sealserver.cpp

LDPATH=${HOME}/.local/lib/
#LDPATH=.
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lsealemulator -lseal
#LDLIB=-lemulator
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}

all: ${TARGET} sealclient

${TARGET}: ${SOURCE}
	${CXX} -o ${TARGET} ${SOURCE} ${LDLIB}

sealclient: sealclient.cpp
	${CXX} -o sealclient $^ ${LDLIB}

clean:
	rm -f ${TARGET} sealclient
