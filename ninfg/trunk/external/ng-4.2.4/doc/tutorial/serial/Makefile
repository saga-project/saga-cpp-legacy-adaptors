# Makefile for PI on just serial program
#
SHELL  = /bin/sh

CLIENT = pi_serial

CC  = gcc

all: $(CLIENT)

$(CLIENT): $(CLIENT).c pi_trial.o
	$(CC) -o $@ $(CLIENT).c pi_trial.o

clean:
	rm -f  $(CLIENT)
	rm -f *~ *.o core core.*
