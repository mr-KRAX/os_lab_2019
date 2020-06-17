CC=gcc
CFLAGS=-I. -std=c11 -pthread
SFLAGS=-lmultModulo -L.
FPFlags=-fPIC -c

all: client server

again: clean all

server: libmultModulo
	$(CC) -o server server.c  $(CFLAGS) $(SFLAGS)

client: libmultModulo
	$(CC) -o client client.c  $(CFLAGS) $(SFLAGS)

libmultModulo: multModulo.o
	$(CC) -shared -o libmultModulo.so multModulo.o $(CFLAGS)

multModulo.o: multModulo.c multModulo.h
	$(CC) -o multModulo.o multModulo.c $(CFLAGS) $(FPFlags)

clean:
	rm multModulo.o libmultModulo.so client server