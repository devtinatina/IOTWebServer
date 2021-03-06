#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o stems.o clientGet.o clientPost.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall
LIBS = -lpthread 
DB = $(shell mysql_config --libs)
DBB = $(shell mysql_config --cflags)
DBBB = -L/usr/local/opt/openssl/lib

.SUFFIXES: .c .o 

all: server clientPost clientGet dataGet.cgi dataPost.cgi

server: server.o request.o stems.o
	$(CC) $(CFLAGS) -o server server.o request.o stems.o $(LIBS)

clientGet: clientGet.o stems.o
	$(CC) $(CFLAGS) -o clientGet clientGet.o stems.o

clientPost: clientPost.o stems.o
	$(CC) $(CFLAGS) -o clientPost clientPost.o stems.o $(LIBS)

dataGet.cgi: dataGet.c stems.o
	$(CC) $(CFLAGS) -o dataGet.cgi dataGet.c stems.o $(DB) $(DBBB)

dataPost.cgi: dataPost.c stems.h stems.o 
	$(CC) $(CFLAGS) -o dataPost.cgi dataPost.c stems.o $(DB) $(DBBB)
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

server.o: stems.h request.h
clientGet.o: stems.h
clientPost.o: stems.h

clean:
	-rm -f $(OBJS) server clientPost clientGet dataGet.cgi dataPost.cgi
