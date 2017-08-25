CC = gcc
CFLAGS = -g -Wall

all: jacobi Jacobi

jacobi: jacobi.o
	${CC} ${CFLAGS} -o jacobi jacobi.o -lpthread
jacobi.o: jacobi.c
	${CC} ${CFLAGS} -c jacobi.c
Jacobi:
	javac -cp . *.java
	jar cfm Jacobi.jar manifest.mf *.class
clean:
	/bin/rm -f *.o
