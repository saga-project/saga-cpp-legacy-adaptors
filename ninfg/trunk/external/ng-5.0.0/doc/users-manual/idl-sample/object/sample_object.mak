# This file 'sample_object.mak' was created by /home/tashiro/ngdvl/ng2/p/bin/ng_gen. Don't edit

CC = gcc
include $(NG_DIR)/lib/template.mk

# CompileOptions:
NG_USER_CFLAGS = 

#  Define NG_COMPILER & NG_LINKER as $(CC) if it is not defined.

NG_COMPILER ?= $(CC)
NG_LINKER ?= $(CC)

# stub sources

NG_STUB_SRC = _stub_sample_object.c 

# stub programs

NG_STUB_PROGRAM = _stub_sample_object

# stub inf files
NG_INF_FILES = _stub_sample_object.inf
# temporary NRF dummy target.
NRF_DUMMY = nrf_dummy

all: $(NG_STUB_PROGRAM) $(NG_INF_FILES) $(NRF_DUMMY)

_stub_sample_object.o: _stub_sample_object.c
	$(NG_COMPILER) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) $(NG_CPPFLAGS) -c _stub_sample_object.c

_stub_sample_object: _stub_sample_object.o sample.o
	$(NG_LINKER) $(NG_CFLAGS) $(CFLAGS) -o _stub_sample_object _stub_sample_object.o $(LDFLAGS) $(NG_STUB_LDFLAGS) sample.o $(LIBS) -lm

_stub_sample_object.inf: _stub_sample_object
	 ./_stub_sample_object -i _stub_sample_object.inf




$(NRF_DUMMY): $(NG_INF_FILES)
	$(NG_DIR)/bin/ng_gen_nrf $(NG_STUB_PROGRAM)

clean:
	rm -f _stub_sample_object.o _stub_sample_object.c

veryclean: clean
	rm -f $(NG_STUB_PROGRAM) $(NG_INF_FILES) *.nrf
	rm -f sample.o
	rm -f _stub_*-info


# END OF Makefile
