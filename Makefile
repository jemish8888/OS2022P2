CC=gcc
CFLAGS=-Wall -g

all: master slave

bakery.o: bakery.c config.h
	$(CC) $(CFLAGS) -c bakery.c

master: master.c bakery.o
	$(CC) $(CFLAGS) -o master master.c bakery.o

slave: slave.c bakery.o
	$(CC) $(CFLAGS) -o slave slave.c bakery.o

clean:
	rm -f master slave bakery.o logfile.* cstest logfile
