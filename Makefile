CC=gcc
LIB_DIR=lib
OBJECTS=${LIB_DIR}/*.o
CFLAGS=-Wall
PROGRAM=hpsa
TEST=test

all: main.o ${OBJECTS}
	${CC} -o ${PROGRAM} $^ ${CFLAGS}

${OBJECTS}:
	cd ${LIB_DIR} && ${CC} -c ${CFLAGS} *.c

test: test.o ${OBJECTS}
	${CC} -o $@ $^

.PHONY: clean
clean:
	rm ${PROGRAM} ${TEST} *.o ${OBJECTS} -f