LIBS=cjson
LDLIBS=$(LIBS:%=-l%)
SOFLAGS=-shared
OBJFLAGS=-g -Wall -c -fPIC
CXX=g++

SEAL_SO_DEP=seal.o symbolic.o sealutl.o chain.o frame.o trace.o core.o

TARGET=seal.so emulator.so sealsym.so sealscript.so

all: ${TARGET}


#For GILES
gileseal.a: seal.o emulator.o
	ar crf gileseal.a *.o

#Project management.
clean:
	rm -f *.o *.so *~ *.a .*.swp

rebuild:
	make clean;
	make;

indent:
	indent *.c *.h
	mkdir -p backup
	mv *~ backup

ctags:
	ctags-universal *.c *.h

submit: indent ctags
	git status


#Building rules.
%.o: %.c %.h Makefile
	${CXX} ${OBJFLAGS} $< 

seal.so: ${SEAL_SO_DEP}
	ld -relocatable $? -o sealbasic.o
	${CXX} ${SOFLAGS} sealbasic.o -o $@ ${LDLIBS}

emulator.so: seal.so emulator.o
	${CXX} ${SOFLAGS} $? -o $@ #-lseal ${LDLIBS}

sealsym.so: seal.so sealsym.o
	${CXX} ${SOFLAGS} $? -o $@

sealscript.so: sealscript.o sealutl.o chain.o
	ld -relocatable $? -o sealbasic.o
	${CXX} ${SOFLAGS} $? -o $@
