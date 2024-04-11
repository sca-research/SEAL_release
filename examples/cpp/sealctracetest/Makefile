#TARGET=gentrace readtrace dymreadtrace
TARGET=gentrace readtrace
#SOURCE=gentrace.cpp readtrace.cpp dymreadtrace.cpp
SOURCE=gentrace.cpp readtrace.cpp

LDPATH=${HOME}/.local/lib/
#LDPATH=.
INCLUDEPATH=${HOME}/.local/include/
LDLIB=-lsealemulator -lseal
#LDLIB=-lemulator
CXXFLAGS =-g -Wall -I${INCLUDEPATH} -L${LDPATH}
CXX=g++ ${CXXFLAGS}
TESTTRACEFILE="/tmp/sealtesttrace.stc"

all: ${TARGET}

gentrace: gentrace.cpp
	${CXX} -o $@ $< ${LDLIB}

readtrace: readtrace.cpp
	${CXX} -o $@ $< ${LDLIB}

#Obseleted
#dymreadtrace: dymreadtrace.cpp
#	${CXX} -o $@ $< ${LDLIB}


valgrind: ${TARGET}
	valgrind -s --leak-check=full gentrace ${TESTTRACEFILE}
	valgrind -s --leak-check=full readtrace ${TESTTRACEFILE}
	valgrind -s --leak-check=full dymreadtrace ${TESTTRACEFILE}

clean:
	rm -f ${TARGET} *~
