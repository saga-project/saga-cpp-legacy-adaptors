# Makefile for PI on multi servers
#

CLIENT = pi_client_2servers
MODULE = pi

NG_CC  = $(NG_DIR)/bin/ng_cc
NG_GEN = $(NG_DIR)/bin/ng_gen

all: $(CLIENT) stub

install:
	make -f $(MODULE).mak install

$(CLIENT): $(CLIENT).c
	$(NG_CC) -o $@ $(CLIENT).c

stub: $(MODULE).idl
	$(NG_GEN) $(MODULE).idl
	make -f $(MODULE).mak

clean:
	if test -f ./$(MODULE).mak; then \
		make -f $(MODULE).mak veryclean; \
	fi
	rm -f  $(MODULE).mak $(CLIENT) $(MODULE):*
	rm -f *~ *.o core core.*
